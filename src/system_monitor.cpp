//
// Created by RGAA on 2024/3/26.
//

#include "system_monitor.h"
#include "gr_context.h"
#include "app_messages.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "tc_controller/vigem_driver_manager.h"
#include "tc_controller/vigem/sdk/ViGEm/Client.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/win32/process_helper.h"
#include "manager/gr_server_manager.h"
#include "manager/run_game_manager.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#pragma comment(lib, "version.lib")
#pragma comment(lib, "kernel32.lib")

namespace tc
{

    std::shared_ptr<SystemMonitor> SystemMonitor::Make(const std::shared_ptr<GrContext>& ctx) {
        return std::make_shared<SystemMonitor>(ctx);
    }

    SystemMonitor::SystemMonitor(const std::shared_ptr<GrContext>& ctx) {
        this->ctx_ = ctx;
    }

    void SystemMonitor::Start() {
        vigem_driver_manager_ = VigemDriverManager::Make();
        msg_listener_ = ctx_->GetMessageNotifier()->CreateListener();
        RegisterMessageListener();

        if (!CheckViGEmDriver()) {
            InstallViGem(true);
        }

        monitor_thread_ = std::make_shared<Thread>([=, this]() {
            while (!exit_) {
                // check vigem
                {
                    bool vigem_installed = CheckViGEmDriver();
                    if (vigem_installed) {
                        if (!TryConnectViGEmDriver()) {
                            NotifyViGEnState(false);
                        } else {
                            NotifyViGEnState(true);
                        }
                    } else {
                        NotifyViGEnState(false);
                    }
                }

                // check server alive or not
                {
                    auto resp = this->CheckServerAlive();
                    if (resp.ok_) {
                        if (resp.value_) {
                            LOGI("server is already running...");
                            ctx_->SendAppMessage(MsgServerAlive {
                                .alive_ = true,
                            });
                        } else {
                            LOGI("server is not running, we'll start it.");
                            this->StartServer();
                            // check again
                            ctx_->PostDelayTask([=, this](){
                                // UI thread
                                ctx_->PostTask([=, this]() {
                                    // Background thread
                                    auto resp = this->CheckServerAlive();
                                    auto started = resp.ok_ && resp.value_;
                                    ctx_->SendAppMessage(MsgServerAlive {
                                        .alive_ = started,
                                    });
                                });
                            }, 250);
                        }
                    } else {
                        // todo
                        LOGW("Check server alive failed by process, check listening port instead.");
                    }
                }

                // check running game
                {
                    auto rgm = ctx_->GetRunGameManager();
                    rgm->CheckRunningGame();
                }

                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }, "", false);
    }

    void SystemMonitor::Exit() {
        exit_ = true;
        if (monitor_thread_->IsJoinable()) {
            monitor_thread_->Join();
        }
    }

    bool SystemMonitor::CheckViGEmDriver() {
        DWORD major = 0, minor = 0;
        std::wstring path = L"C:\\Windows\\System32\\drivers\\ViGEmBus.sys";

        if (GetFileVersion(path, major, minor)) {
            LOGI("ViGEmBus: {}.{}", major, minor);
            if (major > 1 || (major == 1 && minor >= 17)) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    bool SystemMonitor::GetFileVersion(const std::wstring& filePath, DWORD& major, DWORD& minor){
        DWORD dummy;
        DWORD size = GetFileVersionInfoSizeW(filePath.c_str(), &dummy);
        if (size == 0) {
            return false;
        }

        std::vector<BYTE> data(size);
        if (!GetFileVersionInfoW(filePath.c_str(), 0, size, data.data())) {
            return false;
        }

        VS_FIXEDFILEINFO* fileInfo = nullptr;
        UINT fileInfoSize;
        if (!VerQueryValueW(data.data(), L"\\", reinterpret_cast<void**>(&fileInfo), &fileInfoSize)) {
            return false;
        }

        major = (fileInfo->dwFileVersionMS >> 16) & 0xffff;
        minor = (fileInfo->dwFileVersionLS >> 16) & 0xffff;
        return true;
    }

    bool SystemMonitor::TryConnectViGEmDriver() {
        // driver seems already exists, try to connect
        if (!connect_vigem_success_) {
            connect_vigem_success_ = vigem_driver_manager_->TryConnect();
        }

        if (!connect_vigem_success_) {
            LOGI("connect failed.");
            return false;
        } else {
            LOGI("already tested, connect to vigem success.");
            return true;
        }
    }

    void SystemMonitor::InstallViGem(bool silent) {
        auto exe_folder_path = boost::filesystem::initial_path<boost::filesystem::path>().string();
        StringExt::Replace(exe_folder_path, R"(\)", R"(/)");

        std::string cmd;
        if (silent) {
            cmd = std::format("{}/ViGEmBus_1.22.0_x64_x86_arm64.exe /passive /promptrestart", exe_folder_path);
        } else {
            cmd = std::format("{}/ViGEmBus_1.22.0_x64_x86_arm64.exe", exe_folder_path);
        }

        if (!ProcessUtil::StartProcessInWorkDir(exe_folder_path, cmd, {})) {
            LOGE("Install ViGEm device failed.");
        }
    }

    void SystemMonitor::NotifyViGEnState(bool ok) {
        static bool first_emit_state = true;
        auto task = [=, this]() {
            ctx_->SendAppMessage(MsgViGEmState {
                .ok_ = ok,
            });
        };

        if (first_emit_state) {
            first_emit_state = false;
            ctx_->PostDelayTask([=]() {
                task();
            }, 250);
        } else {
            task();
        }
    }

    void SystemMonitor::RegisterMessageListener() {
        msg_listener_ = ctx_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgInstallViGEm>([=, this](const MsgInstallViGEm& msg) {
            ctx_->PostTask([this]() {
                tc::SystemMonitor::InstallViGem(false);
            });
        });
    }

    Response<bool, bool> SystemMonitor::CheckServerAlive() {
        auto resp = Response<bool, bool>::Make(false, false);
        auto processes = ProcessHelper::GetProcessList();
        if (processes.empty()) {
            return resp;
        }
        resp.ok_ = true;

        for (auto& p : processes) {
            //LOGI("p.exe_name: {}", p.exe_name_);
            if (p.exe_name_.find("GammaRayServer.exe") != std::string::npos) {
                resp.value_ = true;
                LOGI("Yes, find it.");
                break;
            }
        }
        return resp;
    }

    void SystemMonitor::StartServer() {
        auto srv_mgr = ctx_->GetServerManager();
        srv_mgr->Start();
    }

}
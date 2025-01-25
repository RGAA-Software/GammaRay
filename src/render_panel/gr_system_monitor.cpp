//
// Created by RGAA on 2024/3/26.
//

#include "gr_system_monitor.h"
#include "gr_context.h"
#include "gr_application.h"
#include "app_messages.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "tc_controller/vigem_driver_manager.h"
#include "tc_controller/vigem/sdk/ViGEm/Client.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/win32/process_helper.h"
#include "gr_render_controller.h"
#include "gr_run_game_manager.h"
#include "render_panel/network/ws_server.h"
#include "gr_settings.h"
#include "service/service_manager.h"
#include <QApplication>

#pragma comment(lib, "version.lib")
#pragma comment(lib, "kernel32.lib")

namespace tc
{

    std::shared_ptr<GrSystemMonitor> GrSystemMonitor::Make(const std::shared_ptr<GrApplication>& app) {
        return std::make_shared<GrSystemMonitor>(app);
    }

    GrSystemMonitor::GrSystemMonitor(const std::shared_ptr<GrApplication>& app) {
        this->app_ = app;
        this->ctx_ = app->GetContext();
        this->service_manager_ = ctx_->GetServiceManager();
    }

    void GrSystemMonitor::Start() {
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
                if (false) {
                    auto resp = this->CheckServerAlive();
                    if (resp.ok_) {
                        if (resp.value_) {
                            //LOGI("server is already running...");
                            ctx_->SendAppMessage(MsgServerAlive {
                                .alive_ = true,
                            });
                        } else {
                            LOGI("server is not running, we'll start it.");
                            //this->StartServer();
                            // check again
                            ctx_->PostUIDelayTask([=, this]() {
                                ctx_->PostTask([=, this]() {
                                    auto resp = this->CheckServerAlive();
                                    auto started = resp.ok_ && resp.value_;
                                    ctx_->SendAppMessage(MsgServerAlive{
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
                    ctx_->PostTask([=, this]() {
                        rgm->CheckRunningGame();
                        auto msg = rgm->GetRunningGamesAsProto();
                        auto ws_server = app_->GetWSServer();
                        if (ws_server) {
                            ws_server->PostBinaryMessage(msg);
                        }

                        auto game_ids = rgm->GetRunningGameIds();
                        ctx_->SendAppMessage(MsgRunningGameIds {
                            .game_ids_ = game_ids,
                        });
                    });
                }

                // check service status
                {
                    auto status = service_manager_->QueryStatus();
                    ctx_->SendAppMessage(MsgServiceAlive {
                        .alive_ = (status == ServiceStatus::kRunning),
                    });
                    //LOGI("Service Status: {}", (int)status);
                }

                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        }, "", false);
    }

    void GrSystemMonitor::Exit() {
        exit_ = true;
        if (monitor_thread_->IsJoinable()) {
            monitor_thread_->Join();
        }
    }

    bool GrSystemMonitor::CheckViGEmDriver() {
        DWORD major = 0, minor = 0;
        std::wstring path = L"C:\\Windows\\System32\\drivers\\ViGEmBus.sys";

        if (GetFileVersion(path, major, minor)) {
            //LOGI("ViGEmBus: {}.{}", major, minor);
            if (major > 1 || (major == 1 && minor >= 17)) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    bool GrSystemMonitor::GetFileVersion(const std::wstring& filePath, DWORD& major, DWORD& minor){
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

    bool GrSystemMonitor::TryConnectViGEmDriver() {
        // driver seems already exists, try to connect
        if (!connect_vigem_success_) {
            connect_vigem_success_ = vigem_driver_manager_->TryConnect();
        }

        if (!connect_vigem_success_) {
            LOGI("connect failed.");
            return false;
        } else {
            //LOGI("already tested, connect to vigem success.");
            return true;
        }
    }

    void GrSystemMonitor::InstallViGem(bool silent) {
        auto exe_folder_path = GrContext::GetCurrentExeFolder();
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

    void GrSystemMonitor::NotifyViGEnState(bool ok) {
        static bool first_emit_state = true;
        auto task = [=, this]() {
            ctx_->SendAppMessage(MsgViGEmState {
                .ok_ = ok,
            });
        };

        if (first_emit_state) {
            first_emit_state = false;
            ctx_->PostUIDelayTask([=]() {
                task();
            }, 250);
        } else {
            task();
        }
    }

    void GrSystemMonitor::RegisterMessageListener() {
        msg_listener_ = ctx_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgInstallViGEm>([=, this](const MsgInstallViGEm& msg) {
            ctx_->PostTask([this]() {
                tc::GrSystemMonitor::InstallViGem(false);
            });
        });

        msg_listener_->Listen<MsgConnectedToService>([=, this](const MsgConnectedToService& msg) {
            StartServer();
        });
    }

    Response<bool, bool> GrSystemMonitor::CheckServerAlive() {
        auto resp = Response<bool, bool>::Make(false, false);
        auto processes = ProcessHelper::GetProcessList(false);
        if (processes.empty()) {
            return resp;
        }
        resp.ok_ = true;

        LOGI("-------------------------------------------------------------------");
        for (auto& p : processes) {
            LOGI("p.exe_name: {}", p->exe_full_path_);
            if (p->exe_full_path_.find(kGammaRayRenderName) != std::string::npos) {
                resp.value_ = true;
                //LOGI("Yes, find it.");
                break;
            }
        }
        return resp;
    }

    void GrSystemMonitor::StartServer() {
        auto srv_mgr = ctx_->GetRenderController();
        srv_mgr->StartServer();
    }

}
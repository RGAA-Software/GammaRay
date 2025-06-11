//
// Created by RGAA on 2024/3/26.
//

#include "gr_system_monitor.h"
#include "gr_context.h"
#include "gr_application.h"
#include "gr_app_messages.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "tc_controller/vigem_driver_manager.h"
#include "tc_controller/vigem/sdk/ViGEm/Client.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/win32/process_helper.h"
#include "gr_render_controller.h"
#include "gr_run_game_manager.h"
#include "render_panel/network/ws_panel_server.h"
#include "gr_settings.h"
#include "service/service_manager.h"
#include "tc_spvr_client/spvr_manager.h"
#include "tc_common_new/http_base_op.h"
#include "devices/profile_api.h"
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
        this->context_ = app->GetContext();
        this->service_manager_ = context_->GetServiceManager();
        this->settings_ = GrSettings::Instance();
        this->spvr_mgr_ = this->context_->GetSpvrManager();
    }

    void GrSystemMonitor::Start() {
        vigem_driver_manager_ = VigemDriverManager::Make();
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        RegisterMessageListener();

        if (!CheckViGEmDriver()) {
            InstallViGem(true);
        }

        monitor_thread_ = std::make_shared<Thread>([=, this]() {
            while (!exit_) {
                // check system servers
                context_->PostTask([=, this]() {
                    this->CheckOnlineServers();
                });

                // verify device id / pwd
                context_->PostTask([=, this]() {
                    auto r = ProfileApi::VerifyDeviceInfo(settings_->device_id_, settings_->device_random_pwd_, settings_->device_safety_pwd_);
                    if (r != ProfileVerifyResult::kVfSuccessRandomPwd && r != ProfileVerifyResult::kVfSuccessSafetyPwd) {
                        LOGE("device id and device random pwd are not pair, will request new pair!");
                        // force update id
                        context_->SendAppMessage(MsgForceRequestDeviceId{});
                    }
                });

                // check vigem
                context_->PostTask([=, this]() {
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
                });

                // check server alive or not
                if (false) {
                    auto resp = this->CheckServerAlive();
                    if (resp.ok_) {
                        if (resp.value_) {
                            //LOGI("server is already running...");
                            context_->SendAppMessage(MsgServerAlive {
                                .alive_ = true,
                            });
                        } else {
                            LOGI("server is not running, we'll start it.");
                            //this->StartServer();
                            // check again
                            context_->PostUIDelayTask([=, this]() {
                                context_->PostTask([=, this]() {
                                    auto resp = this->CheckServerAlive();
                                    auto started = resp.ok_ && resp.value_;
                                    context_->SendAppMessage(MsgServerAlive{
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
                context_->PostTask([=, this]() {
                    auto rgm = context_->GetRunGameManager();
                    rgm->CheckRunningGame();
                    auto msg = rgm->GetRunningGamesAsProto();
                    auto ws_server = app_->GetWsPanelServer();
                    if (ws_server) {
                        ws_server->PostPanelMessage(msg);
                    }

                    auto game_ids = rgm->GetRunningGameIds();
                    context_->SendAppMessage(MsgRunningGameIds {
                        .game_ids_ = game_ids,
                    });
                });

                // check service status
                context_->PostTask([=, this]() {
                    auto status = service_manager_->QueryStatus();
                    context_->SendAppMessage(MsgServiceAlive {
                        .alive_ = (status == ServiceStatus::kRunning),
                    });
                    //LOGI("Service Status: {}", (int)status);
                });

                std::this_thread::sleep_for(std::chrono::seconds(5));
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
            //LOGI("connect failed.");
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
            context_->SendAppMessage(MsgViGEmState {
                .ok_ = ok,
            });
        };

        if (first_emit_state) {
            first_emit_state = false;
            context_->PostUIDelayTask([=]() {
                task();
            }, 250);
        } else {
            task();
        }
    }

    void GrSystemMonitor::RegisterMessageListener() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgInstallViGEm>([=, this](const MsgInstallViGEm& msg) {
            context_->PostTask([this]() {
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
        auto srv_mgr = context_->GetRenderController();
        srv_mgr->StartServer();
    }

    void GrSystemMonitor::CheckOnlineServers() {
        if (!this->VerifyOnlineServers()) {
            auto ret_online_servers = spvr_mgr_->GetOnlineServers();
            if (!ret_online_servers) {
                SpvrError err = ret_online_servers.error();
                LOGE("Can't request online servers: {}:{}, err: {}",
                     settings_->spvr_server_host_, settings_->spvr_server_port_, SpvrError2String(err));
                return;
            }
            auto online_servers = ret_online_servers.value();
            bool settings_changed = false;
            if (!online_servers->relay_servers_.empty()) {
                auto srv = online_servers->relay_servers_.at(0);
                settings_->SetRelayServerHost(srv.srv_w3c_ip_);
                settings_->SetRelayServerPort(srv.srv_working_port_);
                LOGI("Got Relay server: {}:{}", srv.srv_w3c_ip_, srv.srv_working_port_);
                settings_changed = true;
            }
            if (!online_servers->pr_servers_.empty()) {
                auto srv = online_servers->pr_servers_.at(0);
                settings_->SetProfileServerHost(srv.srv_w3c_ip_);
                settings_->SetProfileServerPort(srv.srv_working_port_);
                LOGI("Got Profile server: {}:{}", srv.srv_w3c_ip_, srv.srv_working_port_);
                settings_changed = true;
            }
            if (settings_changed) {
                context_->SendAppMessage(MsgSettingsChanged{});
            }
        }
    }

    bool GrSystemMonitor::VerifyOnlineServers() {
        if (settings_->spvr_server_host_.empty() || settings_->spvr_server_port_.empty()
            || settings_->relay_server_host_.empty() || settings_->relay_server_port_.empty()
            || settings_->profile_server_host_.empty() || settings_->profile_server_port_.empty()) {
            return false;
        }
        // check spvr
        auto ok = HttpBaseOp::CanPingServer(settings_->spvr_server_host_, settings_->spvr_server_port_);
        if (!ok) {
            LOGE("Spvr is not online: {} {} ", settings_->spvr_server_host_, settings_->spvr_server_port_);
            return false;
        }
        //LOGI("Verify Spvr ok, address: {}:{}", settings_->spvr_server_host_, settings_->spvr_server_port_);

        // check relay
        ok = HttpBaseOp::CanPingServer(settings_->relay_server_host_, settings_->relay_server_port_);
        if (!ok) {
            LOGE("Relay is not online: {} {} ", settings_->relay_server_host_, settings_->relay_server_port_);
            return false;
        }
        //LOGI("Verify Relay ok, address: {}:{}", settings_->relay_server_host_, settings_->relay_server_port_);

        // check profile
        ok = HttpBaseOp::CanPingServer(settings_->profile_server_host_, settings_->profile_server_port_);
        if (!ok) {
            LOGE("Profile is not online: {} {} ", settings_->profile_server_host_, settings_->profile_server_port_);
            return false;
        }
        //LOGI("Verify Profile ok, address: {}:{}", settings_->profile_server_host_, settings_->profile_server_port_);

        return true;
    }

}
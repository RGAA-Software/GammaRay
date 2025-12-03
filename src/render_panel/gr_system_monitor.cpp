//
// Created by RGAA on 2024/3/26.
//

#include "gr_system_monitor.h"
#include <qdir.h>
#include <qfileinfo.h>
#include <QApplication>
#include "gr_context.h"
#include "gr_application.h"
#include "gr_app_messages.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "tc_controller/vigem_driver_manager.h"
#include "tc_controller/vigem/sdk/ViGEm/Client.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/string_util.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/win32/process_helper.h"
#include "gr_render_controller.h"
#include "gr_run_game_manager.h"
#include "render_panel/network/ws_panel_server.h"
#include "gr_settings.h"
#include "service/service_manager.h"
#include "tc_spvr_client/spvr_device_api.h"
#include "tc_spvr_client/spvr_device.h"
#include "tc_common_new/http_base_op.h"
#include "tc_common_new/cpu_frequency.h"
#include "tc_profile_client/profile_api.h"
#include "tc_qt_widget/tc_dialog.h"
#include "tc_qt_widget/translator/tc_translator.h"
#include "companion/panel_companion.h"
#include "skin/interface/skin_interface.h"

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
    }

    void GrSystemMonitor::Start() {
        CheckServiceAlive();
        // install service
        this->service_manager_->Install();

        vigem_driver_manager_ = VigemDriverManager::Make();
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        RegisterMessageListener();

        if (!CheckViGEmDriver()) {
            InstallViGem(true);
        }

        monitor_thread_ = std::make_shared<Thread>([=, this]() {
            while (!exit_) {
                // check system servers
                if (settings_->HasSpvrServerConfig()) {
                    context_->PostTask([=, this]() {
                        // this->CheckOnlineServers();
                        this->CheckThisDeviceInfo();
                    });
                }

                // check vigem
                context_->PostTask([=, this]() {
                    bool vigem_installed = CheckViGEmDriver();
                    if (vigem_installed) {
                        if (!TryConnectViGEmDriver()) {
                            NotifyViGEnState(false);
                        }
                        else {
                            NotifyViGEnState(true);
                        }
                    }
                    else {
                        NotifyViGEnState(false);
                    }
                });

                // check running game
                auto skin = grApp->GetSkin();
                if (skin->IsGameEnabled()) {
                    context_->PostTask([=, this]() {
                        auto rgm = context_->GetRunGameManager();
                        rgm->CheckRunningGame();
                        auto msg = rgm->GetRunningGamesAsProto();
                        auto ws_server = app_->GetWsPanelServer();
                        if (ws_server) {
                            ws_server->PostPanelMessage(msg);
                        }

                        auto game_ids = rgm->GetRunningGameIds();
                        context_->SendAppMessage(MsgRunningGameIds{
                            .game_ids_ = game_ids,
                        });
                    });
                }

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
        }, "sys_monitor", false);
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
            cmd = std::format("{}/joystick.exe /passive /promptrestart", exe_folder_path);
        } else {
            cmd = std::format("{}/joystick.exe", exe_folder_path);
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
                tc::GrSystemMonitor::InstallViGem(true);
            });
        });

        msg_listener_->Listen<MsgConnectedToService>([=, this](const MsgConnectedToService& msg) {
            StartServer();
        });

        msg_listener_->Listen<MsgGrTimer1S>([=, this](const MsgGrTimer1S& msg) {
            context_->PostTask([=, this]() {
                // cpu frequency
                auto freq = CpuFrequency::GetCurrentCpuSpeed();
                {
                    std::lock_guard<std::mutex> guard(cpu_frequency_mtx_);
                    this->current_cpu_frequency_.push_back(freq);
                    if (this->current_cpu_frequency_.size() >= 180) {
                        this->current_cpu_frequency_.pop_front();
                    }
                }
                if (auto companion = app_->GetCompanion(); companion) {
                    companion->UpdateCurrentCpuFrequency((float)freq);
                }
            });
        });
    }

    Response<bool, bool> GrSystemMonitor::CheckRenderAlive() {
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

    void GrSystemMonitor::CheckServiceAlive() {
        tc::ServiceStatus serv_status = this->service_manager_->QueryStatus();
        if (tc::ServiceStatus::kUnknownStatus == serv_status) {
            return;
        }

        auto serv_exe_path_res = this->service_manager_->GetServiceExecutablePath();

        if (!serv_exe_path_res.has_value()) {
            LOGE("cant not get serv_exe_path");
            return;
        }

        std::string serv_exe_path = serv_exe_path_res.value();

        QString exe_full_qpath = QString::fromStdString(serv_exe_path);
        QFileInfo file_info(exe_full_qpath);
        QDir parent_dir = file_info.dir();
        std::string serv_parent_path = parent_dir.absolutePath().toStdString();
        serv_parent_path = StringUtil::StandardizeWinPath(serv_parent_path);

        QDir cur_exe_dir = QCoreApplication::applicationDirPath();
        std::string cur_exe_parent_path = cur_exe_dir.absolutePath().toStdString();
        cur_exe_parent_path = StringUtil::StandardizeWinPath(cur_exe_parent_path);

        LOGI("*************************************");
        LOGI("serv_parent_path: {}", serv_parent_path);
        LOGI("cur_exe_parent_path: {}", cur_exe_parent_path);

        if (serv_parent_path != cur_exe_parent_path) {
            uint32_t cur_pid = QCoreApplication::applicationPid();
            QString msg = QString("{path: %1}").arg(QString::fromStdString(serv_parent_path));
            TcDialog dialog(tcTr("id_tips"), tcTr("id_run_other_service_instances") + " ? " + msg, nullptr);
            if (QDialog::Accepted != dialog.exec()) {
                tc::ProcessHelper::CloseProcess(cur_pid);
                return;
            }
            this->service_manager_->Remove(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            auto processes = tc::ProcessHelper::GetProcessList(false);
            for (auto& process : processes) {
                if (process->exe_full_path_.find(kGammaRayGuardName) != std::string::npos) {
                    LOGI("Kill exe: {}", process->exe_full_path_);
                    tc::ProcessHelper::CloseProcess(process->pid_);
                    break;
                }
            }
            for (auto& process : processes) {
                if (process->exe_full_path_.find(kGammaRayClientInner) != std::string::npos) {
                    LOGI("Kill exe: {}", process->exe_full_path_);
                    tc::ProcessHelper::CloseProcess(process->pid_);
                    break;
                }
            }
            for (auto& process : processes) {
                if (process->exe_full_path_.find(kGammaRayRenderName) != std::string::npos) {
                    LOGI("Kill exe: {}", process->exe_full_path_);
                    tc::ProcessHelper::CloseProcess(process->pid_);
                    break;
                }
            }
            for (auto& process : processes) {
                if (process->exe_full_path_.find(kGammaRayName) != std::string::npos) {
                    LOGI("Kill exe: {}", process->exe_full_path_);
                    if (cur_pid != process->pid_) {
                        tc::ProcessHelper::CloseProcess(process->pid_);
                    }
                }
            }
        }
    }

    void GrSystemMonitor::StartServer() {
        auto srv_mgr = context_->GetRenderController();
        srv_mgr->StartServer();
    }

    void GrSystemMonitor::CheckThisDeviceInfo() {
        //LOGI("CheckThisDeviceInfo...");
        // profile server
        auto has_pr_server = HttpBaseOp::CanPingServer(true, settings_->GetSpvrServerHost(), settings_->GetSpvrServerPort(), grApp->GetAppkey());
        if (!has_pr_server) {
            return;
        }

        // don't have device id, force to update
        if (settings_->GetDeviceId().empty() && has_pr_server) {
            context_->SendAppMessage(MsgForceRequestDeviceId{});
            return;
        }

        // has a device
        auto opt_device = spvr::SpvrDeviceApi::QueryDevice(settings_->GetSpvrServerHost(),
                                                     settings_->GetSpvrServerPort(),
                                                     grApp->GetAppkey(),
                                                     settings_->GetDeviceId());
        if (!opt_device.has_value()) {
            if (auto err = opt_device.error(); err == spvr::SpvrApiError::kDeviceNotFound) {
                LOGI("Don't have device in server, id: {}, will request a new one.", settings_->GetDeviceId());
                context_->SendAppMessage(MsgForceRequestDeviceId{});
            }
            return;
        }
        auto device = opt_device.value();
        if (!device) {
            LOGE("Query device for : {} failed.", settings_->GetDeviceId());
            return;
        }

        auto local_random_pwd_md5 = MD5::Hex(settings_->GetDeviceRandomPwd());
        if (device->random_pwd_md5_ != local_random_pwd_md5) {
            LOGW("***Random pwd not equals, will refresh, srv: {} => local: {}", device->random_pwd_md5_, local_random_pwd_md5);
            auto opt_update_device = spvr::SpvrDeviceApi::UpdateRandomPwd(settings_->GetSpvrServerHost(),
                                                                    settings_->GetSpvrServerPort(),
                                                                    grApp->GetAppkey(),
                                                                    settings_->GetDeviceId());
            if (opt_update_device.has_value()) {
                auto update_device =  opt_update_device.value();
                if (update_device && !update_device->gen_random_pwd_.empty()) {
                    settings_->SetDeviceRandomPwd(update_device->gen_random_pwd_);
                    // todo: notify random password updated

                }
            }
        }

        auto current_device_security_pwd = settings_->GetDeviceSecurityPwd();
        if (device->safety_pwd_md5_ != settings_->GetDeviceSecurityPwd() && !current_device_security_pwd.empty()) {
            LOGW("***Safety pwd not equals, will refresh, srv: {} => local: {}", device->safety_pwd_md5_, current_device_security_pwd);
            // update safety password
            auto update_device = spvr::SpvrDeviceApi::UpdateSafetyPwd(settings_->GetSpvrServerHost(),
                                                                settings_->GetSpvrServerPort(),
                                                                grApp->GetAppkey(),
                                                                settings_->GetDeviceId(),
                                                                current_device_security_pwd);
            if (!update_device) {
                LOGE("***UpdateSafetyPwd failed for : {}, SPWD: {}", settings_->GetDeviceId(), current_device_security_pwd);
            }
            else {
                LOGI("***UpdateSafetyPwd success {}, SPWD: {}", settings_->GetDeviceId(), current_device_security_pwd);
            }
        }
    }

    std::vector<double> GrSystemMonitor::GetCurrentCpuFrequency() {
        std::lock_guard<std::mutex> guard(cpu_frequency_mtx_);
        std::vector<double> frequencies;
        for (const auto& v : current_cpu_frequency_) {
            frequencies.push_back(v);
        }
        return frequencies;
    }

}
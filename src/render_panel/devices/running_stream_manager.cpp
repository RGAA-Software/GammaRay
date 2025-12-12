//
// Created by RGAA on 28/03/2025.
//

#include "running_stream_manager.h"
#include <QApplication>
#include <qstandardpaths.h>
#include "tc_common_new/base64.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_context.h"
#include "tc_common_new/log.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/gr_application.h"
#include "tc_profile_client/profile_api.h"
#include "tc_qt_widget/tc_dialog.h"
#include "start_stream_loading.h"
#include "tc_qt_widget/translator/tc_translator.h"
#include "client/ct_stream_item_net_type.h"
#include "render_panel/spvr/gr_spvr_manager.h"

namespace tc
{

    RunningStreamManager::RunningStreamManager(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
        settings_ = GrSettings::Instance();
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgClientConnectedPanel>([=, this](const MsgClientConnectedPanel& msg) {
            // clear loading dialog
            context_->PostUIDelayTask([=, this]() {
                if (loading_dialogs_.contains(msg.stream_id_)) {
                    loading_dialogs_[msg.stream_id_]->hide();
                }
            }, 200);
        });
    }

    void RunningStreamManager::StartStream(const std::shared_ptr<spvr::SpvrStream>& item, const std::string& network_type) {
        // loading dialog
        auto loading = std::make_shared<StartStreamLoading>(context_, item, network_type);
        loading->setWindowFlag(Qt::WindowStaysOnTopHint, true);
        loading->show();
        auto stream_id = item->stream_id_;
        loading_dialogs_.insert({stream_id, loading});
        QTimer::singleShot(7000, [=, this]() {
            if (loading_dialogs_.contains(stream_id)) {
                loading_dialogs_[stream_id]->hide();
                loading_dialogs_.erase(stream_id);
            }
        });

        if (grApp->GetSkinName() != "OpenSource" && !item->remote_device_id_.empty()) {
            auto ac = context_->GetSpvrManager()->QueryNewConnection(true);
            if (ac == std::nullopt) {
                LOGE("Not available connection for : {}", item->remote_device_id_);
                return;
            }
            auto c = ac.value();
            if (!c.available_) {
                const QString msg = tcTr("id_no_available_connection");
                TcDialog dialog(tcTr("id_error"), msg);
                dialog.exec();
                return;
            }
        }

        std::string screen_recording_path = settings_->GetScreenRecordingPath();
        if (screen_recording_path.empty()) {
            QString movies_path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
            screen_recording_path = movies_path.toStdString();
            settings_->SetScreenRecordingPath(screen_recording_path);
        }

        bool show_watermark = true;
        if (grApp->GetSkinName() == "OpenSource" || !item->remote_device_id_.empty()) {
            show_watermark = false;
        }

        // start it
        auto process = std::make_shared<QProcess>();
        QStringList arguments;
        arguments
            << std::format("--host={}", item->stream_host_).c_str()
            << std::format("--port={}", item->stream_port_).c_str()
            << std::format("--appkey={}", grApp->GetAppkey()).c_str()
            << std::format("--spvr_host={}", settings_->GetSpvrServerHost()).c_str()
            << std::format("--spvr_port={}", settings_->GetSpvrServerPort()).c_str()
            << std::format("--audio={}", item->audio_enabled_).c_str()
            << std::format("--clipboard={}", item->clipboard_enabled_).c_str()
            << std::format("--stream_id={}", item->stream_id_).c_str()
            << std::format("--conn_type={}", item->connect_type_).c_str()
            << std::format("--network_type={}", network_type).c_str()
            << std::format("--stream_name={}", Base64::Base64Encode(item->stream_name_)).c_str()
            << std::format("--device_id={}", settings_->GetDeviceId()).c_str()
            << std::format("--device_rp={}", Base64::Base64Encode(settings_->GetDeviceRandomPwd())).c_str()
            << std::format("--device_sp={}", Base64::Base64Encode(settings_->GetDeviceSecurityPwd())).c_str()
            << std::format("--remote_device_id={}", item->remote_device_id_).c_str()
            << std::format("--remote_device_rp={}", Base64::Base64Encode(item->remote_device_random_pwd_)).c_str()
            << std::format("--remote_device_sp={}", Base64::Base64Encode(item->remote_device_safety_pwd_)).c_str()
            << std::format("--enable_p2p={}", item->enable_p2p_).c_str()
            << std::format("--show_max_window={}", settings_->IsMaxWindowEnabled() ? 1 : 0).c_str()
            << std::format("--display_name={}", [=, this]() -> std::string {
                if (network_type == kStreamItemNtTypeRelay) {
                    return settings_->GetDeviceId();
                }
                else {
                    return "My Computer";
                }
            }()).c_str()
            << std::format("--display_remote_name={}", [=, this]() -> std::string {
                if (network_type == kStreamItemNtTypeRelay) {
                    return item->remote_device_id_;
                }
                else {
                    return item->stream_name_.empty() ? item->stream_host_ : item->stream_name_;
                }
            } ()).c_str()
            << std::format("--panel_server_port={}", settings_->GetPanelServerPort()).c_str()
            << std::format("--screen_recording_path={}", screen_recording_path).c_str()
            << std::format("--my_host={}", [=, this]() -> std::string {
                auto ips = context_->GetIps();
                if (!ips.empty()) {
                    return ips[0].ip_addr_;
                }
                return "";
            }()).c_str()
            << std::format("--language={}", (int)tcTrMgr()->GetSelectedLanguage()).c_str()
            << std::format("--only_viewing={}", item->only_viewing_).c_str()
            << std::format("--split_windows={}", item->split_windows_).c_str()
            << std::format("--max_num_of_screen={}", settings_->GetMaxNumOfScreen()).c_str()
            << std::format("--display_logo={}", settings_->IsClientLogoDisplaying() ? 1 : 0).c_str()
            << std::format("--develop_mode={}", settings_->IsDevelopMode() ? 1 : 0).c_str()
            << std::format("--titlebar_color={}", settings_->IsColorfulTitleBarEnabled() ? item->bg_color_ : -1).c_str()
            << std::format("--decoder={}", settings_->GetPreferDecoder()).c_str()
            << std::format("--relay_host={}", item->relay_host_).c_str()
            << std::format("--relay_port={}", item->relay_port_).c_str()
            << std::format("--relay_appkey={}", item->relay_appkey_).c_str()
            << std::format("--force_software={}", item->force_software_ ? 1 : 0).c_str()
            << std::format("--wait_debug={}", item->wait_debug_ ? 1 : 0).c_str()
            << std::format("--force_gdi_capture={}", item->force_gdi_capture_ ? 1 : 0).c_str()
            << std::format("--show_watermark={}", show_watermark ? 1 : 0).c_str()
            ;
        LOGI("Start client inner args:");
        for (auto& arg : arguments) {
            LOGI("{}", arg.toStdString());
        }
        LOGI("MY RDM PWD: {}", item->device_random_pwd_);
        LOGI("RE RDM PWD: {}", item->remote_device_random_pwd_);

        auto client_inner_path = qApp->applicationDirPath() + "/" + kGammaRayClientInner.c_str();
        process->start(client_inner_path, arguments);
        running_processes_.erase(item->stream_id_);
        running_processes_.insert({item->stream_id_, process});
        LOGI("After start client: {}", client_inner_path.toStdString());
    }

    void RunningStreamManager::StopStream(const std::shared_ptr<spvr::SpvrStream>& item) {
        context_->SendAppMessage(ClearWorkspace {
            .item_ = item,
        });
        if (running_processes_.contains(item->stream_id_)) {
            auto process = running_processes_[item->stream_id_];
            if (process) {
                TcDialog dialog(tcTr("id_warning"), tcTr("id_exit_client"), nullptr);
                if (dialog.exec() == kDoneOk) {
                    process->kill();
                    running_processes_.erase(item->stream_id_);
                }
            }
        }
    }

}

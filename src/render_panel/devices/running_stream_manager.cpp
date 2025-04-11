//
// Created by RGAA on 28/03/2025.
//

#include "running_stream_manager.h"
#include "tc_common_new/base64.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_context.h"
#include "tc_common_new/log.h"
#include "tc_dialog.h"
#include "render_panel/gr_app_messages.h"

namespace tc
{

    RunningStreamManager::RunningStreamManager(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
        settings_ = GrSettings::Instance();
    }

    void RunningStreamManager::StartStream(const StreamItem& item) {
        auto process = std::make_shared<QProcess>();
        QStringList arguments;
        arguments
            << std::format("--host={}", item.stream_host_).c_str()
            << std::format("--port={}", item.stream_port_).c_str()
            << std::format("--audio={}", item.audio_enabled_).c_str()
            << std::format("--clipboard={}", item.clipboard_enabled_).c_str()
            << std::format("--stream_id={}", item.stream_id_).c_str()
            << std::format("--conn_type={}", item.connect_type_).c_str()
            << std::format("--network_type={}", item.network_type_).c_str()
            << std::format("--stream_name={}", Base64::Base64Encode(item.stream_name_)).c_str()
            << std::format("--device_id={}", settings_->device_id_).c_str()
            << std::format("--device_rp={}", Base64::Base64Encode(settings_->device_random_pwd_)).c_str()
            << std::format("--device_sp={}", Base64::Base64Encode(settings_->device_safety_pwd_)).c_str()
            << std::format("--remote_device_id={}", item.remote_device_id_).c_str()
            << std::format("--remote_device_rp={}", Base64::Base64Encode(item.remote_device_random_pwd_)).c_str()
            << std::format("--remote_device_sp={}", Base64::Base64Encode(item.remote_device_safety_pwd_)).c_str()
        ;
        LOGI("Start client inner args:");
        for (auto& arg : arguments) {
            LOGI("{}", arg.toStdString());
        }
        LOGI("MY RDM PWD: {}", item.device_random_pwd_);
        LOGI("RE RDM PWD: {}", item.remote_device_random_pwd_);

        process->start("./GammaRayClientInner.exe", arguments);
        running_processes_.erase(item.stream_id_);
        running_processes_.insert({item.stream_id_, process});
    }

    void RunningStreamManager::StopStream(const tc::StreamItem& item) {
        context_->SendAppMessage(ClearWorkspace {
            .item_ = item,
        });
        if (running_processes_.contains(item.stream_id_)) {
            auto process = running_processes_[item.stream_id_];
            if (process) {
                auto dlg = TcDialog::Make("Stop Stream", "Do you want to stop the stream ?", nullptr);
                dlg->SetOnDialogSureClicked([=, this]() {
                    process->kill();
                    running_processes_.erase(item.stream_id_);
                });
                dlg->show();
            }
        }
    }

}
//
// Created by RGAA on 23/05/2025.
//

#include "ct_plugin_event_router.h"
#include "ct_settings.h"
#include "ct_client_context.h"
#include "ct_plugin_events.h"
#include "tc_common_new/log.h"
#include "ct_app_message.h"
#include "ct_base_workspace.h"
#include "tc_client_sdk_new/thunder_sdk.h"
#include "tc_common_new/md5.h"

namespace tc
{

    ClientPluginEventRouter::ClientPluginEventRouter(const std::shared_ptr<BaseWorkspace>& ws) {
        ws_ = ws;
        context_ = ws_->GetContext();
        settings_ = Settings::Instance();
    }

    void ClientPluginEventRouter::ProcessPluginEvent(const std::shared_ptr<ClientPluginBaseEvent>& event) {
        if (!thunder_sdk_) {
            thunder_sdk_ = ws_->GetThunderSdk();
        }
        if (ClientPluginEventType::kPluginTestEvent == event->event_type_) {
            auto target_event = std::dynamic_pointer_cast<ClientPluginTestEvent>(event);
        }
        else if (ClientPluginEventType::kPluginNotifyMsgEvent == event->event_type_) {
            auto target_event = std::dynamic_pointer_cast<ClientPluginNotifyMsgEvent>(event);
            this->context_->NotifyAppMessage(QString::fromStdString(target_event->title_),
                                             QString::fromStdString(target_event->message_),
                                             [=]() {
                                                 if (target_event->clicked_cbk_) {
                                                     target_event->clicked_cbk_();
                                                 }
                                             }
            );
        }
        else if (ClientPluginEventType::kPluginClipboardEvent == event->event_type_) {
            auto target_event = std::dynamic_pointer_cast<ClientPluginClipboardEvent>(event);
            if (target_event->type_ == ClipboardType::kClipboardText) {
                context_->SendAppMessage(MsgClientClipboard {
                    .type_ = target_event->type_,
                    .msg_ = target_event->text_msg_,
                });
            }
            else if (target_event->type_ == ClipboardType::kClipboardFiles) {
                context_->SendAppMessage(MsgClientClipboard {
                    .type_ = target_event->type_,
                    .files_ = target_event->cp_files_,
                });
            }
        }
        else if (ClientPluginEventType::kPluginNetworkEvent == event->event_type_) {
            auto target_event = std::dynamic_pointer_cast<ClientPluginNetworkEvent>(event);
            if (target_event->media_channel_) {
                LOGI("Will post to media channel.");
                thunder_sdk_->PostMediaMessage(target_event->buf_);
            }
            else {
                thunder_sdk_->PostFileTransferMessage(target_event->buf_);
            }
        }
        else if (ClientPluginEventType::kPluginFileTransBeginEvent == event->event_type_) {
            auto target_event = std::dynamic_pointer_cast<ClientPluginFileTransferBeginEvent>(event);
            context_->SendAppMessage(MsgClientFileTransmissionBegin {
                .the_file_id_ = MD5::Hex(target_event->task_id_),
                .begin_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp(),
                .direction_ = target_event->direction_,
                .file_detail_ = target_event->file_path_,
                .remote_device_id_ = settings_->remote_device_id_.empty() ? settings_->host_ : settings_->remote_device_id_,
            });
        }
        else if (ClientPluginEventType::kPluginFileTransferEndEvent == event->event_type_) {
            auto target_event = std::dynamic_pointer_cast<ClientPluginFileTransferEndEvent>(event);
            context_->SendAppMessage(MsgClientFileTransmissionEnd {
                .the_file_id_ = MD5::Hex(target_event->task_id_),
                .end_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp(),
                .duration_ = 0,
                .success_ = target_event->success_,
            });
        }
        else if (ClientPluginEventType::kPluginRemoteClipboardResp == event->event_type_) {
            auto target_event = std::dynamic_pointer_cast<ClientPluginRemoteClipboardResp>(event);
            tc::Message resp_msg;
            resp_msg.set_type(tc::kClipboardInfoResp);
            resp_msg.set_device_id(settings_->device_id_);
            resp_msg.set_stream_id(settings_->stream_id_);
            auto resp_sub = resp_msg.mutable_clipboard_info_resp();
            resp_sub->set_type(ClipboardType::kClipboardText);
            resp_sub->set_msg(target_event->remote_info_);
            thunder_sdk_->PostMediaMessage(resp_msg.SerializeAsString());
            LOGI("send clipboard info resp.");
        }
    }

}
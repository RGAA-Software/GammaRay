//
// Created by RGAA on 23/05/2025.
//

#include "ct_plugin_event_router.h"
#include "ct_client_context.h"
#include "ct_plugin_events.h"
#include "tc_common_new/log.h"
#include "ct_app_message.h"
#include "ct_base_workspace.h"
#include "tc_client_sdk_new/thunder_sdk.h"

namespace tc
{

    ClientPluginEventRouter::ClientPluginEventRouter(const std::shared_ptr<BaseWorkspace>& ws) {
        ws_ = ws;
        context_ = ws_->GetContext();
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
                                             QString::fromStdString(target_event->message_));
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
                thunder_sdk_->PostMediaMessage(target_event->buf_);
            }
            else {
                thunder_sdk_->PostFileTransferMessage(target_event->buf_);
            }
        }
    }

}
//
// Created by RGAA on 23/05/2025.
//

#include "ct_plugin_event_router.h"
#include "ct_client_context.h"
#include "ct_plugin_events.h"
#include "tc_common_new/log.h"

namespace tc
{

    ClientPluginEventRouter::ClientPluginEventRouter(const std::shared_ptr<ClientContext>& ctx) {
        context_ = ctx;
    }

    void ClientPluginEventRouter::ProcessPluginEvent(const std::shared_ptr<ClientPluginBaseEvent>& event) {
        if (ClientPluginEventType::kPluginTestEvent == event->event_type_) {
            auto target_event = std::dynamic_pointer_cast<ClientPluginTestEvent>(event);
            LOGI("test event, callback: {}", target_event->message_);
        }
        else if (ClientPluginEventType::kPluginNotifyMsgEvent == event->event_type_) {
            auto target_event = std::dynamic_pointer_cast<ClientPluginNotifyMsgEvent>(event);
            if (!this->context_) {
                return;
            }
            this->context_->NotifyAppMessage(QString::fromStdString(target_event->title_), QString::fromStdString(target_event->message_));
        }
    }

}
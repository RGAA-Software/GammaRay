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
        if (event->event_type_ == ClientPluginEventType::kPluginTestEvent) {
            auto target_event = std::dynamic_pointer_cast<ClientPluginTestEvent>(event);
            LOGI("test event, callback: {}", target_event->message_);
        }
    }

}
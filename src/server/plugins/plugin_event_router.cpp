//
// Created by RGAA on 15/11/2024.
//

#include "plugin_event_router.h"
#include "tc_common_new/log.h"
#include "plugin_interface/gr_plugin_events.h"

namespace tc
{

    PluginEventRouter::PluginEventRouter(const std::shared_ptr<Context>& ctx) {

    }

    void PluginEventRouter::ProcessPluginEvent(const std::shared_ptr<GrPluginBaseEvent>& event) {
        LOGI("from plugin: {}", event->plugin_name_);
    }

}

//
// Created by RGAA on 15/11/2024.
//

#include "plugin_event_router.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "plugin_interface/gr_plugin_events.h"
#include "plugin_interface/gr_stream_plugin.h"
#include "plugin_interface/gr_encoder_plugin.h"
#include "plugin_manager.h"
#include "plugin_stream_event_router.h"
#include "context.h"
#include <fstream>

namespace tc
{

    PluginEventRouter::PluginEventRouter(const std::shared_ptr<Context>& ctx) {
        context_ = ctx;
        plugin_manager_ = context_->GetPluginManager();
        stream_event_router_ = std::make_shared<PluginStreamEventRouter>(ctx);
    }

    void PluginEventRouter::ProcessPluginEvent(const std::shared_ptr<GrPluginBaseEvent>& event) {
        // encoded video frame
        if (event->plugin_type_ == GrPluginEventType::kPluginEncodedVideoFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginEncodedVideoFrameEvent>(event);
            stream_event_router_->ProcessEncodedVideoFrameEvent(target_event);
        }
        else if (event->plugin_type_ == GrPluginEventType::kPluginNetClientEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginNetClientEvent>(event);

        }
    }

}

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
#include "context.h"
#include <fstream>

namespace tc
{

    PluginEventRouter::PluginEventRouter(const std::shared_ptr<Context>& ctx) {
        context_ = ctx;
        plugin_manager_ = context_->GetPluginManager();
    }

    void PluginEventRouter::ProcessPluginEvent(const std::shared_ptr<GrPluginBaseEvent>& event) {
        // encoded video frame
        if (event->plugin_type_ == GrPluginEventType::kPluginEncodedVideoFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginEncodedVideoFrameEvent>(event);

            // stream plugins: Raw frame / Encoded frame
            context_->PostStreamPluginTask([=, this]() {
                plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin *plugin) {
                    plugin->OnEncodedVideoFrame(target_event->type_, target_event->data_, target_event->frame_index_,
                                                target_event->frame_width_, target_event->frame_height_, target_event->key_frame_);
                });
            });
        }
    }

}

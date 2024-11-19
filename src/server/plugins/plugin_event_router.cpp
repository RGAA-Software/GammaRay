//
// Created by RGAA on 15/11/2024.
//

#include "plugin_event_router.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "plugin_interface/gr_plugin_events.h"

#include <fstream>

namespace tc
{

    PluginEventRouter::PluginEventRouter(const std::shared_ptr<Context>& ctx) {

    }

    void PluginEventRouter::ProcessPluginEvent(const std::shared_ptr<GrPluginBaseEvent>& event) {
        // encoded video frame
        LOGI("plugin callback type: {}", (int)event->plugin_type_);
        if (event->plugin_type_ == GrPluginEventType::kPluginEncodedVideoFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginEncodedVideoFrameEvent>(event);
            static std::ofstream file("123123.h264", std::ios::binary);
            file.write(target_event->data_->CStr(), target_event->data_->Size());
        }
    }

}

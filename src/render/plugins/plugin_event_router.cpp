//
// Created by RGAA on 15/11/2024.
//

#include "plugin_event_router.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/image.h"
#include "plugin_interface/gr_plugin_events.h"
#include "plugin_interface/gr_stream_plugin.h"
#include "plugin_interface/gr_encoder_plugin.h"
#include "plugin_manager.h"
#include "plugin_stream_event_router.h"
#include "plugin_net_event_router.h"
#include "tc_capture_new/capture_message.h"
#include "context.h"
#include <fstream>
#include "app.h"

namespace tc
{

    PluginEventRouter::PluginEventRouter(const std::shared_ptr<Application>& app) {
        app_ = app;
        context_ = app->GetContext();
        plugin_manager_ = context_->GetPluginManager();
        stream_event_router_ = std::make_shared<PluginStreamEventRouter>(app);
        net_event_router_ = std::make_shared<PluginNetEventRouter>(app);
        msg_notifier_ = app_->GetContext()->GetMessageNotifier();
    }

    void PluginEventRouter::ProcessPluginEvent(const std::shared_ptr<GrPluginBaseEvent>& event) {
        // encoded video frame
        if (event->plugin_type_ == GrPluginEventType::kPluginEncodedVideoFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginEncodedVideoFrameEvent>(event);
            stream_event_router_->ProcessEncodedVideoFrameEvent(target_event);
        }
        else if (event->plugin_type_ == GrPluginEventType::kPluginNetClientEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginNetClientEvent>(event);
            net_event_router_->ProcessNetEvent(target_event);
        }
        else if (event->plugin_type_ == GrPluginEventType::kPluginClientConnectedEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginClientConnectedEvent>(event);
            net_event_router_->ProcessClientConnectedEvent(target_event);
        }
        else if (event->plugin_type_ == GrPluginEventType::kPluginClientDisConnectedEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginClientDisConnectedEvent>(event);
            net_event_router_->ProcessClientDisConnectedEvent(target_event);
        }
        else if (event->plugin_type_ == GrPluginEventType::kPluginCapturedVideoFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginCapturedVideoFrameEvent>(event);
            msg_notifier_->SendAppMessage(target_event->frame_);
        }
        else if (event->plugin_type_ == GrPluginEventType::kPluginCapturingMonitorInfoEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginCapturingMonitorInfoEvent>(event);
            net_event_router_->ProcessCapturingMonitorInfoEvent(target_event);
        }
        else if (event->plugin_type_ == GrPluginEventType::kPluginCursorEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginCursorEvent>(event);
            msg_notifier_->SendAppMessage(target_event->cursor_info_);
        }
        else if (event->plugin_type_ == GrPluginEventType::kPluginRawVideoFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginRawVideoFrameEvent>(event);
            auto msg = CaptureVideoFrame{};
            msg.frame_width_ = target_event->image_->width;
            msg.frame_height_ = target_event->image_->height;
            msg.frame_index_ = target_event->frame_index_;
            msg.raw_image_ = target_event->image_;
            msg.adapter_uid_ = -1;
            msg_notifier_->SendAppMessage(msg);
        }
    }

}
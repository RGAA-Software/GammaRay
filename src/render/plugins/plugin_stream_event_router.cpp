//
// Created by RGAA on 21/11/2024.
//

#include "plugin_stream_event_router.h"
#include "context.h"
#include "plugin_manager.h"
#include "tc_capture_new/capture_message.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/image.h"
#include "app/app_messages.h"
#include "plugin_interface/gr_stream_plugin.h"
#include "plugin_interface/gr_net_plugin.h"
#include "network/net_message_maker.h"
#include "settings/settings.h"
#include "statistics.h"
#include "app.h"

namespace tc
{
    PluginStreamEventRouter::PluginStreamEventRouter(const std::shared_ptr<Application>& app) {
        app_ = app;
        context_ = app->GetContext();
        plugin_manager_ = context_->GetPluginManager();
        statistics_ = Statistics::Instance();
    }

    void PluginStreamEventRouter::ProcessEncodedVideoFrameEvent(const std::shared_ptr<GrPluginEncodedVideoFrameEvent>& event) {
        CaptureVideoFrame last_capture_video_frame_;
        try {
            last_capture_video_frame_ = std::any_cast<CaptureVideoFrame>(event->extra_);
        } catch(std::exception& e) {
            LOGE("Cast to CaptureVideoFrame failed: {}", e.what());
            return;
        }
        if (!event->data_) {
            LOGE("Encoded data is null!");
            return;
        }

        auto frame_index = event->frame_index_;
        auto key = event->key_frame_;
        uint32_t frame_width = event->frame_width_;
        uint32_t frame_height = event->frame_height_;

        if (event->key_frame_) {
            LOGI("Encoded: frame size:{}, frame index: {}, key frame: {}, size: {}x{}, monitor: {} - ({},{}, {},{})",
                 event->data_->Size(), frame_index, key, frame_width, frame_height, last_capture_video_frame_.display_name_,
                 last_capture_video_frame_.left_, last_capture_video_frame_.top_, last_capture_video_frame_.right_, last_capture_video_frame_.bottom_);
        } else {
            //LOGI("Encoded frame: {}", frame_index);
        }

        MsgVideoFrameEncoded msg {
            .frame_width_ = static_cast<uint32_t>(frame_width),
            .frame_height_ = static_cast<uint32_t>(frame_height),
            .frame_format_ = (uint32_t)event->type_,
            .frame_index_ = frame_index,
            .key_frame_ = key,
            .image_ = Image::Make(event->data_, frame_width, frame_height),
            .monitor_name_ = last_capture_video_frame_.display_name_,
            .monitor_left_ = last_capture_video_frame_.left_,
            .monitor_top_ = last_capture_video_frame_.top_,
            .monitor_right_ = last_capture_video_frame_.right_,
            .monitor_bottom_ = last_capture_video_frame_.bottom_,
        };
        //context_->SendAppMessage(msg);

        auto video_type = [=]() -> tc::VideoType {
            return (Encoder::EncoderFormat)msg.frame_format_ == Encoder::EncoderFormat::kH264 ? tc::VideoType::kNetH264 : tc::VideoType::kNetHevc;
        } ();
        auto net_msg = NetMessageMaker::MakeVideoFrameMsg(video_type, msg.image_->data,msg.frame_index_, msg.frame_width_,
                                                          msg.frame_height_, msg.key_frame_, msg.monitor_name_,
                                                          msg.monitor_left_, msg.monitor_top_, msg.monitor_right_, msg.monitor_bottom_);
        statistics_->fps_video_encode_->Tick();

        // statistics
        Statistics::Instance()->AppendMediaBytes(net_msg.size());

        // plugins: Frame encoded
        plugin_manager_->VisitNetPlugins([&](GrNetPlugin* plugin) {
            plugin->PostProtoMessage(net_msg);
        });

        context_->PostStreamPluginTask([=, this]() {
            plugin_manager_->VisitStreamPlugins([=](GrStreamPlugin *plugin) {
                // stream plugins: Raw frame / Encoded frame
                plugin->OnEncodedVideoFrame(event->type_, event->data_, event->frame_index_,
                                            event->frame_width_, event->frame_height_, event->key_frame_);
            });
        });
    }

}
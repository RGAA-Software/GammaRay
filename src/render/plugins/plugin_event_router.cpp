//
// Created by RGAA on 15/11/2024.
//

#include "plugin_event_router.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/image.h"
#include "plugin_interface/gr_plugin_events.h"
#include "plugin_interface/gr_stream_plugin.h"
#include "plugin_interface/gr_video_encoder_plugin.h"
#include "plugin_manager.h"
#include "plugin_stream_event_router.h"
#include "plugin_net_event_router.h"
#include "tc_capture_new/capture_message.h"
#include "rd_context.h"
#include <fstream>
#include "rd_app.h"
#include "tc_message.pb.h"

namespace tc
{

    PluginEventRouter::PluginEventRouter(const std::shared_ptr<RdApplication>& app) {
        app_ = app;
        context_ = app->GetContext();
        plugin_manager_ = context_->GetPluginManager();
        stream_event_router_ = std::make_shared<PluginStreamEventRouter>(app);
        net_event_router_ = std::make_shared<PluginNetEventRouter>(app);
        msg_notifier_ = app_->GetContext()->GetMessageNotifier();
    }

    void PluginEventRouter::ProcessPluginEvent(const std::shared_ptr<GrPluginBaseEvent>& event) {
        // encoded video frame
        if (event->event_type_ == GrPluginEventType::kPluginEncodedVideoFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginEncodedVideoFrameEvent>(event);
            stream_event_router_->ProcessEncodedVideoFrameEvent(target_event);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginNetClientEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginNetClientEvent>(event);
            net_event_router_->ProcessNetEvent(target_event);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginClientConnectedEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginClientConnectedEvent>(event);
            net_event_router_->ProcessClientConnectedEvent(target_event);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginClientDisConnectedEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginClientDisConnectedEvent>(event);
            net_event_router_->ProcessClientDisConnectedEvent(target_event);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginCapturedVideoFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginCapturedVideoFrameEvent>(event);
            msg_notifier_->SendAppMessage(target_event->frame_);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginCapturingMonitorInfoEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginCapturingMonitorInfoEvent>(event);
            net_event_router_->ProcessCapturingMonitorInfoEvent(target_event);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginCursorEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginCursorEvent>(event);
            msg_notifier_->SendAppMessage(target_event->cursor_info_);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginRawVideoFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginRawVideoFrameEvent>(event);
            auto msg = CaptureVideoFrame{};
            msg.frame_width_ = target_event->image_->width;
            msg.frame_height_ = target_event->image_->height;
            msg.frame_index_ = target_event->frame_index_;
            msg.raw_image_ = target_event->image_;
            msg.adapter_uid_ = -1;
            msg.frame_format_ = target_event->frame_format_;
            msg_notifier_->SendAppMessage(msg);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginRawAudioFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginRawAudioFrameEvent>(event);
            auto msg = CaptureAudioFrame{};
            msg.samples_ = target_event->sample_rate_;
            msg.channels_ = target_event->channels_;
            msg.bits_ = target_event->bits_;
            msg.full_data_ = target_event->full_data_;
            msg_notifier_->SendAppMessage(msg);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginSplitRawAudioFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginSplitRawAudioFrameEvent>(event);
            auto msg = CaptureAudioFrame{};
            msg.samples_ = target_event->sample_rate_;
            msg.channels_ = target_event->channels_;
            msg.bits_ = target_event->bits_;
            msg.left_ch_data_ = target_event->left_ch_data_;
            msg.right_ch_data_ = target_event->right_ch_data_;
            msg_notifier_->SendAppMessage(msg);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginEncodedAudioFrameEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginEncodedAudioFrameEvent>(event);
            net_event_router_->ProcessEncodedAudioFrameEvent(target_event->data_,
                                                             target_event->sample_rate_,
                                                             target_event->channels_,
                                                             target_event->bits_,
                                                             target_event->frame_size_);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginInsertIdrEvent) {
            plugin_manager_->VisitEncoderPlugins([=, this](GrVideoEncoderPlugin* plugin) {
                // TODO:
                //LOGI("Insert IDR for plugin: {}", plugin->GetPluginName());
                plugin->InsertIdr();
            });
        }
        else if (event->event_type_ == GrPluginEventType::kPluginClipboardEvent) {
            // clipboard update
        }
        else if (event->event_type_ == GrPluginEventType::kPluginRelayPausedEvent) {

        }
        else if (event->event_type_ == GrPluginEventType::kPluginRelayResumeEvent) {

        }
        else if (event->event_type_ == GrPluginEventType::kPluginRtcAnswerSdpEvent) {
            this->SendAnswerSdpToRemote(event);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginRtcIceEvent) {
            this->SendIceToRemote(event);
        }
        else if (event->event_type_ == GrPluginEventType::kPluginRtcReportEvent) {
            auto target_event = std::dynamic_pointer_cast<GrPluginRtcReportEvent>(event);
            net_event_router_->ProcessRtcReportEvent(target_event);
        }
    }

    void PluginEventRouter::SendAnswerSdpToRemote(const std::shared_ptr<GrPluginBaseEvent>& event) {
        auto target_event = std::dynamic_pointer_cast<GrPluginRtcAnswerSdpEvent>(event);
        auto stream_id = target_event->stream_id_;

        tc::Message pt_msg;
        pt_msg.set_type(MessageType::kSigAnswerSdpMessage);
        auto sub = pt_msg.mutable_sig_answer_sdp();
        sub->set_sdp(target_event->sdp_);
        auto msg = pt_msg.SerializeAsString();

        plugin_manager_->VisitNetPlugins([=, this](GrNetPlugin* plugin) {
            if (plugin->GetPluginId() == kRelayPluginId) {
                if (stream_id.empty()) {
                    plugin->PostProtoMessage(msg, true);
                }
                else {
                    plugin->PostTargetStreamProtoMessage(stream_id, msg, true);
                }
                LOGI("Send SDP by relay: {}", stream_id);
            }
        });
    }

    void PluginEventRouter::SendIceToRemote(const std::shared_ptr<GrPluginBaseEvent>& event) {
        auto target_event = std::dynamic_pointer_cast<GrPluginRtcIceEvent>(event);
        auto stream_id = target_event->stream_id_;

        tc::Message pt_msg;
        pt_msg.set_type(MessageType::kSigIceMessage);
        auto sub = pt_msg.mutable_sig_ice();
        sub->set_ice(target_event->ice_);
        sub->set_mid(target_event->mid_);
        sub->set_sdp_mline_index(target_event->sdp_mline_index_);
        auto msg = pt_msg.SerializeAsString();

        plugin_manager_->VisitNetPlugins([=, this](GrNetPlugin* plugin) {
            if (plugin->GetPluginId() == kRelayPluginId) {
                if (stream_id.empty()) {
                    plugin->PostProtoMessage(msg, true);
                }
                else {
                    plugin->PostTargetStreamProtoMessage(stream_id, msg, true);
                }
                LOGI("Send ICE by relay: {}", target_event->ice_);
            }
        });
    }

}

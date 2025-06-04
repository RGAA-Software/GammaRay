//
// Created by RGAA on 2024/3/6.
//

#include "rd_statistics.h"
#include "tc_render_panel_message.pb.h"
#include "tc_common_new/log.h"
#include "tc_common_new/fps_stat.h"
#include "app/app_messages.h"
#include "rd_context.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/time_util.h"
#include "plugins/plugin_manager.h"
#include "plugins/plugin_ids.h"
#include "render/rd_app.h"
#include "plugin_interface/gr_monitor_capture_plugin.h"
#include "plugin_interface/gr_video_encoder_plugin.h"
#include "plugin_interface/gr_net_plugin.h"
#include "settings/rd_settings.h"
#include "app/video_frame_carrier.h"

namespace tc
{
    /// ---
    constexpr auto kMaxDurationCount = 180;

    void MsgWorkingCaptureInfo::AppendCopyTextureDuration(int32_t duration) {
        copy_texture_durations_.push_back(duration);
        if (copy_texture_durations_.size() > kMaxDurationCount) {
            copy_texture_durations_.pop_front();
        }
    }

    std::vector<int32_t> MsgWorkingCaptureInfo::GetCopyTextureDurations() {
        std::vector<int32_t> result;
        for (const auto& v : copy_texture_durations_) {
            result.push_back(v);
        }
        return result;
    }

    void MsgWorkingCaptureInfo::AppendMapCvtTextureDuration(int32_t duration) {
        map_cvt_texture_durations_.push_back(duration);
        if (map_cvt_texture_durations_.size() > kMaxDurationCount) {
            map_cvt_texture_durations_.pop_front();
        }
    }

    std::vector<int32_t> MsgWorkingCaptureInfo::GetMapCvtTextureDurations() {
        std::vector<int32_t> result;
        for (const auto& v : map_cvt_texture_durations_) {
            result.push_back(v);
        }
        return result;
    }

    /// ----
    RdStatistics::RdStatistics() {
        settings_ = RdSettings::Instance();
    }

    void RdStatistics::SetApplication(const std::shared_ptr<RdApplication>& app) {
        app_ = app;
        context_ = app_->GetContext();
        plugin_mgr_ = context_->GetPluginManager();
    }

    void RdStatistics::StartMonitor() {
        monitor_thread_ = Thread::Make("render_monitor", 128);
        monitor_thread_->Poll();

        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgTimer5000>([this](const MsgTimer5000& m) {
            this->OnChecking();
        });
    }

    void RdStatistics::Exit() {
        if (monitor_thread_) {
            monitor_thread_->Exit();
        }
    }

    void RdStatistics::IncreaseRunningTime() {
        running_time_++;
    }

    void RdStatistics::AppendMediaBytes(int bytes) {
        send_media_bytes_ += bytes;
    }

    void RdStatistics::AppendAudioFrameGap(uint32_t time) {
        if (audio_frame_gaps_.size() >= kMaxStatCounts) {
            audio_frame_gaps_.erase(audio_frame_gaps_.begin());
        }
        audio_frame_gaps_.push_back(time);
    }

    std::string RdStatistics::AsProtoMessage() {
        tcrp::RpMessage msg;
        msg.set_type(tcrp::RpMessageType::kRpCaptureStatistics);

        auto cst = msg.mutable_capture_statistics();
        cst->mutable_audio_frame_gaps()->Add(audio_frame_gaps_.begin(), audio_frame_gaps_.end());

        // from inner server
        cst->set_app_running_time(running_time_);
        // from inner server
        cst->set_server_send_media_data(send_media_bytes_);
        //
        auto video_capture_plugin = app_->GetWorkingMonitorCapturePlugin();
        auto video_encoder_plugins = app_->GetWorkingVideoEncoderPlugins();
        auto frame_carriers = app_->GetWorkingFrameCarriers();
        if (video_capture_plugin && !video_encoder_plugins.empty()) {
            // encoder info

            auto captures_info = video_capture_plugin->GetWorkingCapturesInfo();
            for (const auto& [name, info] : captures_info) {
                auto cp_info = cst->mutable_working_captures_info();
                auto item = cp_info->Add();
                item->set_target_name(info->target_name_);
                item->set_capturing_fps(info->fps_);
                item->set_capture_type(info->capture_type_);
                auto video_capture_gaps = item->mutable_video_capture_gaps();
                for (const auto& v : info->capture_gaps_) {
                    video_capture_gaps->Add(v);
                }

                // encoder
                if (video_encoder_plugins.contains(info->target_name_)) {
                    auto video_encoder_plugin = video_encoder_plugins[info->target_name_];
                    auto video_encoders_info = video_encoder_plugin->GetWorkingCapturesInfo();
                    if (video_encoders_info.contains(info->target_name_)) {
                        auto encoder_info = video_encoders_info[info->target_name_];
                        item->set_encoder_name(encoder_info->encoder_name_);
                        item->set_encoding_fps(encoder_info->fps_);

                        auto encode_durations = item->mutable_encode_durations();
                        for (const auto& v : encoder_info->encode_durations_) {
                            encode_durations->Add(v);
                        }
                    }
                }
                item->set_capture_frame_width(info->capture_frame_width_);
                item->set_capture_frame_height(info->capture_frame_height_);

                // capture info
                //LOGI("Target name: {}", info->target_name_);
                if (app_captures_info_.contains(info->target_name_)) {
                    auto app_cp_info = CaptureInfo(info->target_name_);
                    //LOGI("copy texture durations: {}", app_cp_info->copy_texture_durations_.size());
                    //LOGI("map&cvt texture durations: {}", app_cp_info->map_cvt_texture_durations_.size());
                    {
                        auto durations = item->mutable_copy_texture_durations();
                        for (const auto &v: app_cp_info->copy_texture_durations_) {
                            durations->Add(v);
                        }
                    }
                    {
                        auto durations = item->mutable_map_cvt_texture_durations();
                        for (const auto& v : app_cp_info->map_cvt_texture_durations_) {
                            durations->Add(v);
                        }
                    }
                }

                // resize info
                if (settings_->encoder_.encode_res_type_ == Encoder::EncodeResolutionType::kOrigin) {
                    item->set_resize_frame_width(0);
                    item->set_resize_frame_height(0);
                }
                else {
                    if (frame_carriers.contains(info->target_name_)) {
                        auto carrier = frame_carriers[info->target_name_];
                        item->set_resize_frame_width(carrier->GetResizeWidth());
                        item->set_resize_frame_height(carrier->GetResizeHeight());
                    }
                }
            }
        }

        int32_t connected_clients = 0;
        int32_t relay_connected_size = 0;
        plugin_mgr_->VisitNetPlugins([&](GrNetPlugin* plugin) {
            if (plugin->GetPluginId() == kRelayPluginId) {
                relay_connected_size = plugin->ConnectedClientSize();
                return;
            }
            connected_clients += plugin->ConnectedClientSize();
        });
        cst->set_connected_clients(connected_clients);

        if (relay_connected_size >= 1) {
            cst->set_relay_connected(true);
        }
        else {
            cst->set_relay_connected(false);
        }

        // audio capture
        cst->set_audio_capture_type("WASAPI");

        //
        cst->set_video_encode_type([=, this]() {
            if (video_encoder_format_ == Encoder::EncoderFormat::kH264) {
                return tcrp::VideoType::kNetH264;
            }
            else if (video_encoder_format_ == Encoder::EncoderFormat::kHEVC) {
                return tcrp::VideoType::kNetHevc;
            }
            return tcrp::VideoType::kNetH264;
        } ());

        cst->set_audio_encode_type(tcrp::AudioEncodeType::kNetOpus);

        return msg.SerializeAsString();
    }

    void RdStatistics::IncreaseDDAFailedCount() {
        dda_failed_count_++;
    }

    void RdStatistics::OnChecking() {
        monitor_thread_->Post([=, this]() {
            //TimeDuration td("ProcessUtil::GetThreadCount");
            auto thread_count = ProcessUtil::GetThreadCount();
            //TODO:
            //LOGI("Thread count: {}", thread_count);
        });
    }

    std::shared_ptr<MsgWorkingCaptureInfo> RdStatistics::CaptureInfo(const std::string& name) {
        if (app_captures_info_.contains(name)) {
            return app_captures_info_[name];
        }
        auto info = std::make_shared<MsgWorkingCaptureInfo>();
        app_captures_info_[name] = info;
        return info;
    }

}

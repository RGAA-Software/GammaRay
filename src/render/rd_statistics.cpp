//
// Created by RGAA on 2024/3/6.
//

#include "rd_statistics.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"
#include "tc_common_new/fps_stat.h"
#include "app/app_messages.h"
#include "rd_context.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/time_ext.h"

namespace tc
{

    RdStatistics::RdStatistics() {
        fps_video_encode_ = std::make_shared<FpsStat>();
    }

    void RdStatistics::SetContext(const std::shared_ptr<RdContext>& ctx) {
        context_ = ctx;
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

    void RdStatistics::AppendEncodeDuration(uint32_t time) {
        if (encode_durations_.size() >= kMaxStatCounts) {
            encode_durations_.erase(encode_durations_.begin());
        }
        encode_durations_.push_back(time);
    }

    void RdStatistics::AppendFrameGap(uint32_t time) {
        if (video_frame_gaps_.size() >= kMaxStatCounts) {
            video_frame_gaps_.erase(video_frame_gaps_.begin());
        }
        video_frame_gaps_.push_back(time);
    }

    void RdStatistics::AppendAudioFrameGap(uint32_t time) {
        if (audio_frame_gaps_.size() >= kMaxStatCounts) {
            audio_frame_gaps_.erase(audio_frame_gaps_.begin());
        }
        audio_frame_gaps_.push_back(time);
    }

    void RdStatistics::TickFps() {
        fps_video_encode_value_ = fps_video_encode_->value();
    }

    std::string RdStatistics::AsProtoMessage() {
        tc::Message msg;
        msg.set_type(tc::MessageType::kCaptureStatistics);

        auto cst = msg.mutable_capture_statistics();
        cst->mutable_video_frame_gaps()->Add(video_frame_gaps_.begin(), video_frame_gaps_.end());
        cst->mutable_encode_durations()->Add(encode_durations_.begin(), encode_durations_.end());
        cst->mutable_audio_frame_gaps()->Add(audio_frame_gaps_.begin(), audio_frame_gaps_.end());

        cst->mutable_decode_durations()->Add(decode_durations_.begin(), decode_durations_.end());
        cst->mutable_client_video_recv_gaps()->Add(client_video_recv_gaps_.begin(), client_video_recv_gaps_.end());
        cst->set_client_fps_video_recv(client_fps_video_recv_);
        cst->set_client_fps_render(client_fps_render_);
        cst->set_client_recv_media_data(client_recv_media_data_);
        // from inner server
        cst->set_fps_video_encode(fps_video_encode_value_);
        // from inner server
        cst->set_app_running_time(running_time_);
        // from inner server
        cst->set_server_send_media_data(send_media_bytes_);
        cst->set_capture_width(capture_width_);
        cst->set_capture_height(capture_height_);
        cst->set_render_width(render_width_);
        cst->set_render_height(render_height_);
        return msg.SerializeAsString();
    }

    void RdStatistics::IncreaseDDAFailedCount() {
        dda_failed_count_++;
    }

    void RdStatistics::OnChecking() {
        monitor_thread_->Post([=, this]() {
            TimeDuration td("ProcessUtil::GetThreadCount");
            auto thread_count = ProcessUtil::GetThreadCount();
            LOGI("Thread count: {}", thread_count);
        });
    }

}

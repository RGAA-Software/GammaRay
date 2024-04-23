//
// Created by RGAA on 2024/3/6.
//

#include "statistics.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"
#include "tc_common_new/fps_stat.h"

namespace tc
{

    Statistics::Statistics() {
        fps_video_encode_ = std::make_shared<FpsStat>();
    }

    void Statistics::IncreaseRunningTime() {
        running_time_++;
    }

    void Statistics::AppendMediaBytes(int bytes) {
        send_media_bytes_ += bytes;
    }

    void Statistics::AppendEncodeDuration(uint32_t time) {
        if (encode_durations_.size() >= kMaxStatCounts) {
            encode_durations_.erase(encode_durations_.begin());
        }
        encode_durations_.push_back(time);
    }

    void Statistics::AppendFrameGap(uint32_t time) {
        if (video_frame_gaps_.size() >= kMaxStatCounts) {
            video_frame_gaps_.erase(video_frame_gaps_.begin());
        }
        video_frame_gaps_.push_back(time);
    }

    void Statistics::AppendAudioFrameGap(uint32_t time) {
        if (audio_frame_gaps_.size() >= kMaxStatCounts) {
            audio_frame_gaps_.erase(audio_frame_gaps_.begin());
        }
        audio_frame_gaps_.push_back(time);
    }

    void Statistics::TickFps() {
        fps_video_encode_value_ = fps_video_encode_->value();
    }

    std::string Statistics::AsProtoMessage() {
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
        return msg.SerializeAsString();
    }

}

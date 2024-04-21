//
// Created by RGAA on 2024/3/6.
//

#include "statistics.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"

namespace tc
{

    void Statistics::IncreaseRunningTime() {
        running_time_++;
    }

    void Statistics::AppendVideoFrameBytes(int bytes) {
        video_frame_bytes_ += bytes;
    }

    void Statistics::AppendAudioFrameBytes(int bytes) {
        audio_frame_bytes_ += bytes;
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

    std::string Statistics::AsProtoMessage() {
        tc::Message msg;
        msg.set_type(tc::MessageType::kCaptureStatistics);

        auto cst = msg.mutable_capture_statistics();
        cst->mutable_video_frame_gaps()->Add(video_frame_gaps_.begin(), video_frame_gaps_.end());
        cst->mutable_encode_durations()->Add(encode_durations_.begin(), encode_durations_.end());
        cst->mutable_decode_durations()->Add(decode_durations_.begin(), decode_durations_.end());
        return msg.SerializeAsString();
    }

}

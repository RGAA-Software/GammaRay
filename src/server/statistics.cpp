//
// Created by RGAA on 2024/3/6.
//

#include "statistics.h"

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

    void Statistics::AppendEncodeTime(uint32_t time) {
        if (encode_times_.size() >= kMaxStatCounts) {
            encode_times_.erase(encode_times_.begin());
        }
        encode_times_.push_back(time);
    }

    void Statistics::AppendFrameGap(uint32_t time) {
        if (video_frame_gaps_.size() >= kMaxStatCounts) {
            video_frame_gaps_.erase(video_frame_gaps_.begin());
        }
        video_frame_gaps_.push_back(time);
    }

}

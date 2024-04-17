//
// Created by hy on 2024/3/6.
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

}

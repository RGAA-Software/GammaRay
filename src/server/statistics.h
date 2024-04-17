//
// Created by hy on 2024/3/6.
//

#ifndef TC_APPLICATION_STATISTICS_H
#define TC_APPLICATION_STATISTICS_H

#include <string>
#include <cstdint>

namespace tc
{

    class Statistics {
    public:

        static Statistics* Instance() {
            static Statistics st;
            return &st;
        }

        void IncreaseRunningTime();
        void AppendVideoFrameBytes(int bytes);
        void AppendAudioFrameBytes(int bytes);

    public:

        // unit: S
        int64_t running_time_{};
        int64_t video_frame_bytes_{};
        int64_t audio_frame_bytes_{};
    };

}

#endif //TC_APPLICATION_STATISTICS_H

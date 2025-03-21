//
// Created by RGAA on 2024/3/6.
//

#ifndef TC_APPLICATION_STATISTICS_H
#define TC_APPLICATION_STATISTICS_H

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <atomic>
#include "tc_common_new/fps_stat.h"

namespace tc
{

    constexpr auto kMaxStatCounts = 180;

    class Context;

    class Statistics {
    public:

        static Statistics* Instance() {
            static Statistics st;
            return &st;
        }

        Statistics();
        void SetContext(const std::shared_ptr<Context>& ctx);
        void IncreaseRunningTime();
        void AppendMediaBytes(int bytes);
        void AppendEncodeDuration(uint32_t time);
        void AppendFrameGap(uint32_t time);
        void AppendAudioFrameGap(uint32_t time);
        void IncreaseDDAFailedCount();

        void TickFps();
        std::string AsProtoMessage();

    public:
        std::shared_ptr<Context> context_ = nullptr;
        // unit: S
        int64_t running_time_{};
        int64_t send_media_bytes_{};
        std::shared_ptr<FpsStat> fps_video_encode_{};
        int fps_video_encode_value_{0};

        std::vector<uint32_t> encode_durations_;
        std::vector<uint32_t> video_frame_gaps_;
        std::vector<uint32_t> audio_frame_gaps_;
        int capture_width_ = 0;
        int capture_height_ = 0;
        std::vector<double> left_spectrum_;
        std::vector<double> right_spectrum_;

        // from client
        std::vector<uint32_t> decode_durations_;
        std::vector<uint32_t> client_video_recv_gaps_;
        uint32_t client_fps_video_recv_ = 0;
        uint32_t client_fps_render_ = 0;
        int64_t client_recv_media_data_ = 0;
        int render_width_ = 0;
        int render_height_ = 0;

        int32_t audio_samples_{0};
        int32_t audio_channels_{0};
        int32_t audio_bits_{0};

        std::atomic_int dda_failed_count_{0};
    };

}

#endif //TC_APPLICATION_STATISTICS_H

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
#include "settings/rd_settings.h"

namespace tc
{

    constexpr auto kMaxStatCounts = 180;

    class RdContext;
    class Thread;
    class MessageListener;
    class PluginManager;
    class RdApplication;

    // video capture/encode info
    class MsgWorkingCaptureInfo {
    public:
        void AppendCopyTextureDuration(int32_t duration);
        std::vector<int32_t> GetCopyTextureDurations();

        void AppendMapCvtTextureDuration(int32_t duration);
        std::vector<int32_t> GetMapCvtTextureDurations();

    public:
        std::deque<int32_t> copy_texture_durations_;
        std::deque<int32_t> map_cvt_texture_durations_;

    };

    class RdStatistics {
    public:

        static RdStatistics* Instance() {
            static RdStatistics st;
            return &st;
        }

        RdStatistics();
        void SetApplication(const std::shared_ptr<RdApplication>& app);
        void StartMonitor();
        void Exit();
        void IncreaseRunningTime();
        void AppendMediaBytes(int bytes);
        void AppendAudioFrameGap(uint32_t time);
        void IncreaseDDAFailedCount();
        std::shared_ptr<MsgWorkingCaptureInfo> CaptureInfo(const std::string& name);

        std::string AsProtoMessage();

    private:
        void OnChecking();

    public:
        std::shared_ptr<RdApplication> app_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<Thread> monitor_thread_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<PluginManager> plugin_mgr_ = nullptr;
        // unit: S
        int64_t running_time_{};
        int64_t send_media_bytes_{};

        Encoder::EncoderFormat video_encoder_format_;

        std::vector<uint32_t> audio_frame_gaps_;
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

        // in app level not in plugins
        std::map<std::string, std::shared_ptr<MsgWorkingCaptureInfo>> app_captures_info_;

    };

}

#endif //TC_APPLICATION_STATISTICS_H

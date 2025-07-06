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
#include "tc_common_new/concurrent_vector.h"
#include "tc_common_new/concurrent_hashmap.h"

namespace tc
{

    constexpr auto kMaxStatCounts = 180;

    class Data;
    class Thread;
    class RdContext;
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
        void CopyLeftSpectrum(const std::vector<double>& sp);
        std::vector<double> GetLeftSpectrum();
        void CopyRightSpectrum(const std::vector<double>& sp);
        std::vector<double> GetRightSpectrum();

        template<typename Collection>
        void CopyDecodeDurations(const Collection& ds) {
            if (decode_durations_.Size() < ds.size()) {
                decode_durations_.Resize(ds.size());
            };
            decode_durations_.CopyMemFrom(ds);
        }

        template<typename Collection>
        void CopyClientVideoRecvGaps(const Collection& gaps) {
            if (client_video_recv_gaps_.Size() < gaps.size()) {
                client_video_recv_gaps_.Resize(gaps.size());
            }
            client_video_recv_gaps_.CopyMemFrom(gaps);
        }

        std::shared_ptr<MsgWorkingCaptureInfo> CaptureInfo(const std::string& name);
        std::shared_ptr<Data> AsProtoMessage();

    private:
        void OnChecking();

    public:
        std::shared_ptr<RdApplication> app_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<Thread> monitor_thread_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<PluginManager> plugin_mgr_ = nullptr;
        RdSettings* settings_ = nullptr;
        // unit: S
        std::atomic_int64_t running_time_{};
        std::atomic_int64_t send_media_bytes_{};

        Encoder::EncoderFormat video_encoder_format_;

        std::atomic_uint32_t client_fps_video_recv_ = 0;
        std::atomic_uint32_t client_fps_render_ = 0;
        std::atomic_int64_t client_recv_media_data_ = 0;
        std::atomic_int32_t render_width_ = 0;
        std::atomic_int32_t render_height_ = 0;

        std::atomic_int32_t audio_samples_{0};
        std::atomic_int32_t audio_channels_{0};
        std::atomic_int32_t audio_bits_{0};

        std::atomic_int dda_failed_count_{0};

    private:
        ConcurrentVector<uint32_t> audio_frame_gaps_;
        ConcurrentVector<double> left_spectrum_;
        ConcurrentVector<double> right_spectrum_;

        // from client
        ConcurrentVector<uint32_t> decode_durations_;
        ConcurrentVector<uint32_t> client_video_recv_gaps_;

        // in renderer app level not in plugins
        ConcurrentHashMap<std::string, std::shared_ptr<MsgWorkingCaptureInfo>> app_captures_info_;

    };

}

#endif //TC_APPLICATION_STATISTICS_H

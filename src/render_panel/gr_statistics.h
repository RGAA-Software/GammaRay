//
// Created by RGAA on 2024-04-20.
//

#ifndef GAMMARAY_GR_STATISTICS_H
#define GAMMARAY_GR_STATISTICS_H

#include <map>
#include <memory>
#include <vector>
#include <cstdint>
#include <string>
#include "tc_message.pb.h"
#include "gr_app_messages.h"

namespace tc
{

    class GrContext;
    class MessageListener;

    class GrStatistics {
    public:

        static GrStatistics* Instance() {
            static GrStatistics instance;
            return &instance;
        }

        void SetContext(const std::shared_ptr<GrContext>& ctx) { context_ = ctx;}
        void RegisterEventListeners();

    private:
        void ProcessCaptureStatistics(const MsgCaptureStatistics& msg);
        void ProcessAudioSpectrum(const MsgServerAudioSpectrum& msg);
        void Process1SCalculation();

    public:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;

        std::map<std::string, std::vector<int32_t>> encode_durations_;
        std::map<std::string, std::vector<int32_t>> video_capture_gaps_;
        std::vector<int32_t> audio_frame_gaps_;
        // from inner server
        int32_t app_running_time = 0;
        // from inner server
        int64_t server_send_media_bytes = 0;
        int64_t last_server_send_media_bytes = 0;
        int64_t send_speed_bytes = 0;

        // from inner server
        int32_t audio_samples_{0};
        int32_t audio_channels_{0};
        int32_t audio_bits_{0};
        std::vector<double> left_spectrum_;
        std::vector<double> right_spectrum_;

        std::vector<tc::PtMsgWorkingCaptureInfo> captures_info_;
        int32_t connected_clients_ = 0;
        std::string video_capture_type_;
        std::string video_encode_type_;
        bool relay_connected_ = false;
        std::string audio_capture_type_;

    };

}

#endif //GAMMARAY_GR_STATISTICS_H

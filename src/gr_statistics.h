//
// Created by RGAA on 2024-04-20.
//

#ifndef GAMMARAY_GR_STATISTICS_H
#define GAMMARAY_GR_STATISTICS_H

#include <memory>
#include <vector>
#include <cstdint>

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

    public:

        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;

        std::vector<uint32_t> video_frame_gaps_;
        std::vector<uint32_t> encode_durations_;
        std::vector<uint32_t> audio_frame_gaps_;
        // from inner server
        int32_t fps_video_encode = 0;
        // from inner server
        int32_t app_running_time = 0;
        // from inner server
        int64_t server_send_media_bytes = 0;

        // from client
        std::vector<uint32_t> decode_durations_;
        std::vector<uint32_t> client_video_recv_gaps_;
        uint32_t client_fps_video_recv_ = 0;
        uint32_t client_fps_render_ = 0;
        int64_t client_recv_media_data_ = 0;

        // from client
        int32_t render_width_ = 0;
        // from client
        int32_t render_height_ = 0;
        // from inner server
        int32_t capture_width_ = 0;
        // from inner server
        int32_t capture_height_ = 0;

    };

}

#endif //GAMMARAY_GR_STATISTICS_H

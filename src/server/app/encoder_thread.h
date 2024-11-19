//
// Created by RGAA on 2023-12-24.
//

#ifndef TC_APPLICATION_ENCODER_THREAD_H
#define TC_APPLICATION_ENCODER_THREAD_H

#include <memory>
#include <functional>
#include "tc_capture_new/capture_message.h"
#include "settings/settings.h"

namespace tc
{
    class Thread;
    class VideoEncoder;
    class Data;
    class Image;
    class File;
    class Context;
    class MessageListener;
    class PluginManager;
    class GrEncoderPlugin;

    class EncoderThread {
    public:
        static std::shared_ptr<EncoderThread> Make(const std::shared_ptr<Context>& ctx);

        explicit EncoderThread(const std::shared_ptr<Context>& ctx);
        ~EncoderThread() = default;

        void Encode(const std::shared_ptr<Data>& data, int width, int height, uint64_t frame_index);
        void Encode(const std::shared_ptr<Image>& image, uint64_t frame_index);
        void Encode(const CaptureVideoFrame& msg);
        void Exit();

    private:
        Settings* settings_ = nullptr;
        std::shared_ptr<Thread> enc_thread_ = nullptr;
        std::shared_ptr<VideoEncoder> video_encoder_ = nullptr;
        std::shared_ptr<Context> context_ = nullptr;

        int frame_width_ = 0;
        int frame_height_ = 0;
        Encoder::EncoderFormat encoder_format_ = Encoder::EncoderFormat::kH264;

        CaptureVideoFrame last_capture_video_frame_{};

        // debug
        std::shared_ptr<File> debug_file_ = nullptr;

        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<PluginManager> plugin_manager_ = nullptr;
        GrEncoderPlugin* working_encoder_plugin_ = nullptr;
    };

}

#endif //TC_APPLICATION_ENCODER_THREAD_H

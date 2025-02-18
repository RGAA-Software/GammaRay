//
// Created by RGAA on 2023-12-24.
//

#ifndef TC_APPLICATION_ENCODER_THREAD_H
#define TC_APPLICATION_ENCODER_THREAD_H

#include <map>
#include <memory>
#include <functional>
#include <optional>
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
    class GrVideoEncoderPlugin;
    class VideoFrameCarrier;
    class Application;

    class EncoderThread {
    public:
        static std::shared_ptr<EncoderThread> Make(const std::shared_ptr<Application>& app);

        explicit EncoderThread(const std::shared_ptr<Application>& app);
        ~EncoderThread() = default;

        void Encode(const std::shared_ptr<Image>& image, uint64_t frame_index);
        void Encode(const CaptureVideoFrame& msg);
        void Exit();
        GrVideoEncoderPlugin* GetWorkingVideoEncoderPlugin();

    private:
        void PostEncTask(std::function<void()>&& task);
        std::shared_ptr<VideoFrameCarrier> GetFrameCarrier(int8_t monitor_idx);

    private:
        Settings* settings_ = nullptr;
        std::shared_ptr<Thread> enc_thread_ = nullptr;
        std::shared_ptr<Context> context_ = nullptr;
        std::shared_ptr<Application> app_ = nullptr;
        uint32_t frame_width_ = 0;
        uint32_t frame_height_ = 0;
        Encoder::EncoderFormat encoder_format_ = Encoder::EncoderFormat::kH264;

        // debug
        std::shared_ptr<File> debug_file_ = nullptr;

        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<PluginManager> plugin_manager_ = nullptr;
        GrVideoEncoderPlugin* working_encoder_plugin_ = nullptr;
        std::map<int8_t, std::shared_ptr<VideoFrameCarrier>> frame_carriers_;
        std::map<int8_t, std::optional<CaptureVideoFrame>> last_video_frames_;
    };

}

#endif //TC_APPLICATION_ENCODER_THREAD_H

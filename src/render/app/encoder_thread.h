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
#include "settings/rd_settings.h"

namespace tc
{
    class Thread;
    class VideoEncoder;
    class Data;
    class Image;
    class File;
    class RdContext;
    class MessageListener;
    class PluginManager;
    class GrVideoEncoderPlugin;
    class VideoFrameCarrier;
    class RdApplication;
    class RdStatistics;

    class EncoderThread {
    public:
        static std::shared_ptr<EncoderThread> Make(const std::shared_ptr<RdApplication>& app);

        explicit EncoderThread(const std::shared_ptr<RdApplication>& app);
        ~EncoderThread() = default;

        void Encode(const std::shared_ptr<Image>& image, uint64_t frame_index);
        void Encode(const CaptureVideoFrame& msg);
        void Exit();
        std::map<std::string, GrVideoEncoderPlugin*> GetWorkingVideoEncoderPlugins();

    private:
        void PostEncTask(std::function<void()>&& task);
        std::shared_ptr<VideoFrameCarrier> GetFrameCarrier(const std::string& monitor_name);
        void PrintEncoderConfig(const tc::EncoderConfig& config);
        bool HasEncoderForMonitor(const std::string& monitor_name);
        GrVideoEncoderPlugin* GetEncoderForMonitor(const std::string& monitor_name);

    private:
        RdSettings* settings_ = nullptr;
        RdStatistics* stat_ = nullptr;
        std::shared_ptr<Thread> enc_thread_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<RdApplication> app_ = nullptr;
        Encoder::EncoderFormat encoder_format_ = Encoder::EncoderFormat::kH264;

        // debug
        std::shared_ptr<File> debug_file_ = nullptr;

        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<PluginManager> plugin_manager_ = nullptr;
        std::map<std::string, GrVideoEncoderPlugin*> encoder_plugins_;
        std::map<std::string, std::shared_ptr<VideoFrameCarrier>> frame_carriers_;
        std::map<std::string, std::optional<CaptureVideoFrame>> last_video_frames_;
    };

}

#endif //TC_APPLICATION_ENCODER_THREAD_H

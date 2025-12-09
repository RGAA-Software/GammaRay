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
#include "tc_encoder_new/encoder_config.h"

namespace tc
{
    class Data;
    class Image;
    class File;
    class Thread;
    class RdContext;
    class RdStatistics;
    class VideoEncoder;
    class RdApplication;
    class PluginManager;
    class MessageListener;
    class GrVideoEncoderPlugin;
    class GrFrameCarrierPlugin;

    class EncoderThread {
    public:
        static std::shared_ptr<EncoderThread> Make(const std::shared_ptr<RdApplication>& app);

        explicit EncoderThread(const std::shared_ptr<RdApplication>& app);
        ~EncoderThread() = default;

        void Encode(const CaptureVideoFrame& msg);
        void Exit();
        std::map<std::string, GrVideoEncoderPlugin*> GetWorkingVideoEncoderPlugins();

    private:
        void PostEncTask(std::function<void()>&& task);
        void PrintEncoderConfig(const tc::EncoderConfig& config);
        bool HasEncoderForMonitor(const std::string& monitor_name);
        GrVideoEncoderPlugin* GetEncoderPluginForMonitor(const std::string& monitor_name);

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
        std::map<std::string, std::optional<CaptureVideoFrame>> last_video_frames_;

        // frame carrier plugin
        GrFrameCarrierPlugin* frame_carrier_plugin_ = nullptr;

        // hardware disabled
        std::atomic_bool hardware_disabled_ = false;

        std::atomic_bool clear_encoders_ = false;
    };

}

#endif //TC_APPLICATION_ENCODER_THREAD_H

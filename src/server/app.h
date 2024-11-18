//
// Created by RGAA on 2023-12-16.
//

#ifndef TC_APPLICATION_TCAPPLICATION_H
#define TC_APPLICATION_TCAPPLICATION_H

#include <string>
#include <memory>
#include <unordered_map>
#include <condition_variable>
#include "tc_common_new/concurrent_hashmap.h"
#include "tc_common_new/concurrent_queue.h"
#include "app_global_messages.h"
#include "app/app_messages.h"
#include "tc_capture_new/capture_message.h"
#include "context.h"
#include <QApplication>

namespace tc
{

    using AppParams = std::unordered_map<std::string, std::string>;

    class Data;
    class Connection;
    class AppManager;
    class HostIpcManager;
    class EncoderThread;
    class MessageListener;
    class DesktopCapture;
    class Thread;
    class AppTimer;
    class Settings;
    class File;
    class AppSharedMessage;
    class IAudioCapture;
    class OpusAudioEncoder;
    class OpusAudioDecoder;
    class CommandManager;
    class ServerCast;
    class AppSharedInfo;
    class Message;
    class CaptureVideoFrame;
    class VigemController;
    class VigemDriverManager;
    class Statistics;
    class WSClient;
    class ClipboardManager;
    class PluginManager;

    class Application : public std::enable_shared_from_this<Application>, public QObject {
    public:

        static std::shared_ptr<Application> Make(const AppParams& args);

        explicit Application(const AppParams& args);
        ~Application() override;

        virtual void Init(int argc, char** argv);
        virtual int Run();
        virtual void Exit();
        virtual void CaptureControlC() = 0;

        void PostGlobalAppMessage(std::shared_ptr<AppMessage>&& msg);
        void PostGlobalTask(std::function<void()>&& task);
        void PostIpcMessage(std::shared_ptr<Data>&& msg);
        void PostIpcMessage(const std::string& msg);
        void PostNetMessage(const std::string& msg);
        std::shared_ptr<Context> GetContext() { return context_; }
        std::shared_ptr<AppManager> GetAppManager() { return app_manager_; }
        std::shared_ptr<ClipboardManager> GetClipboardManager() { return clipboard_mgr_; }
        std::shared_ptr<DesktopCapture> GetDesktopCapture() { return desktop_capture_; }
        void OnIpcVideoFrame(const std::shared_ptr<CaptureVideoFrame>& msg);
        void ProcessGamepadState(const MsgGamepadState& state);
        void ResetMonitorResolution(const std::string& name, int w, int h);

        template<typename T>
        void SendAppMessage(const T& m) {
            context_->SendAppMessage(m);
        }

    private:
        void InitAppTimer();
        void InitMessages();
        void InitGlobalAudioCapture();
        void InitWebRtc();
        void WriteBoostUpInfoForPid(uint32_t pid);
        void StartProcessWithHook();
        void StartProcessWithScreenCapture();
        bool HasConnectedPeer();

        void InitVigemController();
        void ReleaseVigemController();
        void ReportAudioSpectrum();
        void SendClipboardMessage(const std::string& msg);
        void SendConfigurationBack();
        void RequestRestartMe();

        void PostRawImageReaderThread(std::function<void()>&& task);

    protected:
        Settings* settings_ = nullptr;
        std::shared_ptr<WSClient> ws_client_ = nullptr;
        std::shared_ptr<Connection> connection_ = nullptr;
        std::shared_ptr<AppManager> app_manager_ = nullptr;
        std::shared_ptr<Context> context_ = nullptr;
        std::shared_ptr<EncoderThread> encoder_thread_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<DesktopCapture> desktop_capture_ = nullptr;
        std::shared_ptr<AppTimer> app_timer_ = nullptr;

        std::shared_ptr<File> debug_encode_file_ = nullptr;
        std::shared_ptr<AppSharedMessage> app_shared_message_ = nullptr;
        bool exit_app_ = false;

        std::shared_ptr<Thread> audio_capture_thread_ = nullptr;
        std::shared_ptr<IAudioCapture> audio_capture_ = nullptr;
        std::shared_ptr<OpusAudioEncoder> opus_encoder_ = nullptr;
        bool debug_opus_decoder_ = false;
        std::shared_ptr<OpusAudioDecoder> opus_decoder_ = nullptr;

        std::shared_ptr<AppSharedInfo> app_shared_info_ = nullptr;

        std::shared_ptr<Thread> control_thread_ = nullptr;
        std::shared_ptr<VigemController> vigem_controller_ = nullptr;

        uint64_t last_capture_screen_time_ = 0;
        uint64_t last_post_video_time_ = 0;
        uint64_t last_post_audio_time_ = 0;
        Statistics* statistics_ = nullptr;

        std::shared_ptr<Data> audio_cache_ = nullptr;
        int audio_callback_count_ = 0;

        std::shared_ptr<QApplication> qapp_ = nullptr;

        std::shared_ptr<ClipboardManager> clipboard_mgr_ = nullptr;

        std::shared_ptr<PluginManager> plugin_manager_ = nullptr;
    };

    // Windows
    class WinApplication : public Application {
    public:
        explicit WinApplication(const AppParams& args);
        ~WinApplication() override;

        int Run() override;
        void Exit() override;
        //void SendHelloMessageToDll(uint32_t pid) override;
        void CaptureControlC();
        void LoadDxAddress();
    };

}

#endif //TC_APPLICATION_TCAPPLICATION_H

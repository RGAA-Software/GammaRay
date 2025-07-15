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
#include "rd_context.h"
#include <QApplication>
#include <d3d11.h>
#include <wrl/client.h>

namespace tc
{
    using namespace Microsoft::WRL;
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
    class RdSettings;
    class File;
    class AppSharedMessage;
    class IAudioCapture;
    class OpusAudioEncoder;
    class OpusAudioDecoder;
    class ServerCast;
    class AppSharedInfo;
    class Message;
    class CaptureVideoFrame;
    class VigemController;
    class VigemDriverManager;
    class RdStatistics;
    class WsPanelClient;
    class PluginManager;
    class GrMonitorCapturePlugin;
    class GrVideoEncoderPlugin;
    class GrDataProviderPlugin;
    class GrAudioEncoderPlugin;
    class SharedPreference;
    class RenderServiceClient;
    class MonitorRefresher;
    class WinDesktopManager;
    class D3D11DeviceWrapper;

    class RdApplication : public std::enable_shared_from_this<RdApplication>, public QObject {
    public:

        static std::shared_ptr<RdApplication> Make(const AppParams& args);

        explicit RdApplication(const AppParams& args);
        ~RdApplication() override;

        virtual void Init(int argc, char** argv);
        virtual int Run();
        virtual void Exit();
        virtual void CaptureControlC() = 0;

        void PostGlobalAppMessage(std::shared_ptr<AppMessage>&& msg);
        void PostGlobalTask(std::function<void()>&& task);
        void PostIpcMessage(std::shared_ptr<Data>&& msg);
        void PostIpcMessage(const std::string& msg);
        void PostNetMessage(std::shared_ptr<Data> msg);
        std::shared_ptr<RdContext> GetContext() { return context_; }
        std::shared_ptr<AppManager> GetAppManager() { return app_manager_; }
        void OnIpcVideoFrame(const std::shared_ptr<CaptureVideoFrame>& msg);
        void ResetMonitorResolution(const std::string& name, int w, int h);
        std::shared_ptr<PluginManager> GetPluginManager();
        tc::GrMonitorCapturePlugin* GetWorkingMonitorCapturePlugin();
        std::map<std::string, GrVideoEncoderPlugin*> GetWorkingVideoEncoderPlugins();
        bool GenerateD3DDevice(uint64_t adapter_uid);
        ComPtr<ID3D11Device> GetD3DDevice(uint64_t adapter_uid);
        ComPtr<ID3D11DeviceContext> GetD3DContext(uint64_t adapter_uid);
        SharedPreference* GetSp() { return sp_; }
        void ReqCtrlAltDelete(const std::string& device_id, const std::string& stream_id);
        std::shared_ptr<WinDesktopManager> GetDesktopManager();
        // post to panel process
        void PostPanelMessage(std::shared_ptr<Data> msg);

    public:
        template<typename T>
        void SendAppMessage(const T& m) {
            context_->SendAppMessage(m);
        }
    private:
        void InitAppTimer();
        void InitMessages();
        void InitAudioCapture();
        void WriteBoostUpInfoForPid(uint32_t pid);
        void StartProcessWithHook();
        void StartProcessWithScreenCapture();
        bool HasConnectedPeer();

        // to panel
        void ReportAudioSpectrum2Panel();
        // to clients
        void SendAudioSpectrumMessage();
        void SendClipboardMessage(const std::string& msg);
        void SendConfigurationBack();
        void RequestRestartMe();

        void SwitchGdiCapture();

    protected:
        RdSettings* settings_ = nullptr;
        std::shared_ptr<WsPanelClient> ws_panel_client_ = nullptr;
        std::shared_ptr<AppManager> app_manager_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<EncoderThread> encoder_thread_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<AppTimer> app_timer_ = nullptr;

        std::shared_ptr<File> debug_encode_file_ = nullptr;
        std::shared_ptr<AppSharedMessage> app_shared_message_ = nullptr;
        bool exit_app_ = false;

        std::shared_ptr<Thread> audio_capture_thread_ = nullptr;
        std::shared_ptr<AppSharedInfo> app_shared_info_ = nullptr;

        //std::shared_ptr<Thread> control_thread_ = nullptr;
        //std::shared_ptr<VigemController> vigem_controller_ = nullptr;

        uint64_t last_post_audio_time_ = 0;
        RdStatistics* statistics_ = nullptr;
        SharedPreference* sp_ = nullptr;

        std::shared_ptr<QApplication> qapp_ = nullptr;

        std::shared_ptr<PluginManager> plugin_manager_ = nullptr;
        tc::GrMonitorCapturePlugin* monitor_capture_plugin_ = nullptr;
        tc::GrDataProviderPlugin* data_provider_plugin = nullptr;
        tc::GrDataProviderPlugin* audio_capture_plugin_ = nullptr;
        tc::GrAudioEncoderPlugin* audio_encoder_plugin_ = nullptr;

        // uint64_t adapter_uid <==> D3D11Device/D3D11DeviceContext
        std::map<uint64_t, std::shared_ptr<D3D11DeviceWrapper>> d3d11_devices_;

        std::vector<double> fft_left_;
        std::vector<double> fft_right_;

        std::shared_ptr<tc::RenderServiceClient> service_client_ = nullptr;

        // monitor refresher
        std::shared_ptr<MonitorRefresher> monitor_refresher_ = nullptr;

        std::shared_ptr<WinDesktopManager> desktop_mgr_ = nullptr;
    };

    extern std::shared_ptr<RdApplication> rdApp;

    // Windows
    class WinApplication : public RdApplication {
    public:
        explicit WinApplication(const AppParams& args);
        ~WinApplication() override;

        int Run() override;
        void Exit() override;
        void CaptureControlC();
        void LoadDxAddress();
    };

}

#endif //TC_APPLICATION_TCAPPLICATION_H

//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_GR_PLUGIN_INTERFACE_H
#define GAMMARAY_GR_PLUGIN_INTERFACE_H

#include <mutex>
#include <map>
#include <any>
#include <string>
#include <vector>
#include <functional>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QPixmap>
#include <d3d11.h>
#include <mutex>
#include <wrl/client.h>
#include "gr_plugin_settings_info.h"
#include "app/app_messages.h"
#include "tc_capture_new/capture_message.h"

namespace tc
{

    class Data;
    class Image;
    class GrPluginBaseEvent;
    class GrPluginContext;
    class GrNetPlugin;
    class Message;

    // param
    class GrPluginParam {
    public:
        std::map<std::string, std::any> cluster_;
    };

    // plugin type
    enum class GrPluginType {
        kStream,
        kEncoder,
        kNet,
        kUtil,
    };

    // encoded video type
    enum class GrPluginEncodedVideoType {
        kH264,
        kH265,
        kVp8,
        kVp9,
        kAv1
    };

    // callback
    using GrPluginEventCallback = std::function<void(const std::shared_ptr<GrPluginBaseEvent>&)>;

    // interface
    class GrPluginInterface : public QObject {
    public:
        GrPluginInterface() = default;
        ~GrPluginInterface() override = default;

        GrPluginInterface(const GrPluginInterface&) = delete;
        GrPluginInterface& operator=(const GrPluginInterface&) = delete;

        std::shared_ptr<GrPluginContext> GetPluginContext();

        // info
        virtual std::string GetPluginId() = 0;
        virtual std::string GetPluginName();
        virtual std::string GetPluginAuthor();
        virtual std::string GetPluginDescription();
        virtual GrPluginType GetPluginType();
        virtual bool IsStreamPlugin();

        // version
        virtual std::string GetVersionName();
        virtual uint32_t GetVersionCode();

        // enable
        virtual bool IsPluginEnabled();
        virtual void EnablePlugin();
        virtual void DisablePlugin();

        // working
        virtual bool IsWorking();

        // lifecycle
        virtual bool OnCreate(const GrPluginParam& param);
        virtual bool OnResume();
        virtual bool OnStop();
        virtual bool OnDestroy();

        // task
        void PostWorkTask(std::function<void()>&& task);
        void PostUITask(std::function<void()>&& task);
        void PostUIDelayTask(int ms, std::function<void()>&& task);

        // event
        void RegisterEventCallback(const GrPluginEventCallback& cbk);
        void CallbackEvent(const std::shared_ptr<GrPluginBaseEvent>& event);
        void CallbackEventDirectly(const std::shared_ptr<GrPluginBaseEvent>& event);

        virtual void On1Second();

        // key frame
        virtual void InsertIdr();

        virtual void OnCommand(const std::string& command);

        // new client connected
        virtual void OnNewClientConnected(const std::string& visitor_device_id, const std::string& stream_id, const std::string& conn_type);
        // client disconnected
        virtual void OnClientDisconnected(const std::string& visitor_device_id, const std::string& stream_id);

        // widget
        QWidget* GetRootWidget();
        bool eventFilter(QObject *watched, QEvent *event) override;
        void ShowRootWidget();
        void HideRootWidget();

        // net plugins
        void AttachNetPlugin(const std::string& id, GrNetPlugin* plugin);
        // total plugins
        void AttachPlugin(const std::string& id, GrPluginInterface* plugin);

        //
        bool HasAttachedNetPlugins();
        // Serialized proto message from Renderer
        // to see format details in tc_message_new/tc_message.proto
        // such as : message VideoFrame { ... }
        // you can send it to any clients
        //                       -> client 1
        // Renderer Messages ->  -> client 2
        //                       -> client 3
        // run_through: send the message even if stream was paused
        // !! Call this function in a NON-NET-PLUGIN !!
        void DispatchAllStreamMessage(std::shared_ptr<Data> msg, bool run_through = false);

        // Serialized proto message from Renderer
        // to a specific stream
        // !! Call this function in a NON-NET-PLUGIN !!
        void DispatchTargetStreamMessage(const std::string& stream_id, std::shared_ptr<Data> msg, bool run_through = false);

        // Serialized proto message from Renderer
        // to file transfer
        // !! Call this function in a NON-NET-PLUGIN !!
        void DispatchTargetFileTransferMessage(const std::string& stream_id, std::shared_ptr<Data> msg, bool run_through = false);

        // messages from remote
        virtual void OnMessage(std::shared_ptr<Message> msg);
        // msg: Parsed messages
        virtual void OnMessageRaw(const std::any& msg);

        std::map<std::string, GrNetPlugin*> GetNetPlugins();
        int64_t GetQueuingMediaMsgCountInNetPlugins();
        int64_t GetQueuingFtMsgCountInNetPlugins();

        // settings from render panel
        // render panel -> render -> plugins
        virtual void OnSyncPluginSettingsInfo(const GrPluginSettingsInfo& settings);

        // app events
        // render -> plugins
        virtual void DispatchAppEvent(const std::shared_ptr<AppBaseEvent>& event);

        GrPluginSettingsInfo GetPluginSettingsInfo();

        bool DontHaveConnectedClientsNow();

        // update capturing monitors information
        virtual void UpdateCaptureMonitorInfo(const CaptureMonitorInfoMessage& msg);

        // stream
        // video
        virtual void OnVideoEncoderCreated(const std::string& mon_name, const GrPluginEncodedVideoType& type, int width, int height) {}

        // data: encode video frame, h264/h265/...
        virtual void OnEncodedVideoFrame(const std::string& mon_name,
                                         const GrPluginEncodedVideoType& video_type,
                                         const std::shared_ptr<Data>& data,
                                         uint64_t frame_index,
                                         int frame_width,
                                         int frame_height,
                                         bool key) {}
        // raw video frame
        // handle: D3D Shared texture handle
        virtual void OnRawVideoFrameSharedTexture(const std::string& mon_name, uint64_t frame_idx, int frame_width, int frame_height, uint64_t handle, int64_t adapter_id, uint64_t frame_format) {}

        // raw video frame in rgba format
        // image: Raw image
        virtual void OnRawVideoFrameRgba(const std::string& mon_name, uint64_t frame_idx, int frame_width, int frame_height, const std::shared_ptr<Image>& image) {}

        // raw video frame in yuv(I420) format
        // image: Raw image
        virtual void OnRawVideoFrameYuv(const std::string& mon_name, uint64_t frame_idx, int frame_width, int frame_height, const std::shared_ptr<Image>& image) {}

        // audio
        virtual void OnRawAudioData(const std::shared_ptr<Data>& data, int samples, int channels, int bits) {}
        virtual void OnSplitRawAudioData(const std::shared_ptr<Data>& left_ch_data,
                                         const std::shared_ptr<Data>& right_ch_data,
                                         int samples, int channels, int bits) {}
        virtual void OnSplitFFTAudioData(const std::vector<double>& left_fft, const std::vector<double>& right_fft) {}

        GrPluginInterface* GetPluginById(const std::string& plugin_id);
    protected:
        bool HasParam(const std::string& k) {
            return param_.cluster_.count(k) > 0;
        }

        template<typename T>
        T GetConfigParam(const std::string& k) {
            if (param_.cluster_.count(k) > 0) {
                return std::any_cast<T>(param_.cluster_[k]);
            }
            return T{};
        }

        template<typename T>
        bool HoldsType(const std::any& a) {
            return a.type() == typeid(T);
        }

        std::string GetConfigStringParam(const std::string& k) { return GetConfigParam<std::string>(k); }
        int64_t GetConfigIntParam(const std::string& k) { return GetConfigParam<int64_t>(k); }
        bool GetConfigBoolParam(const std::string& k) {return GetConfigParam<bool>(k); }
        double GetConfigDoubleParam(const std::string& k) { return GetConfigParam<double>(k); }

    protected:
        std::shared_ptr<GrPluginContext> plugin_context_ = nullptr;
        std::atomic_bool stopped_ = false;
        std::atomic_bool destroyed_ = false;
        GrPluginParam param_;
        GrPluginEventCallback event_cbk_ = nullptr;
        std::string plugin_file_name_;
        GrPluginType plugin_type_ = GrPluginType::kUtil;
        QWidget* root_widget_ = nullptr;
        std::string plugin_author_;
        std::string plugin_desc_;
        std::string plugin_version_name_;
        int64_t plugin_version_code_;
        bool plugin_enabled_ = true;
        std::string base_path_;
        std::string base_data_path_;
        std::string capture_audio_device_id_;
        // active net plugins...
        std::map<std::string, GrNetPlugin*> net_plugins_;
        // total plugins
        std::map<std::string, GrPluginInterface*> total_plugins_;
        // settings
        GrPluginSettingsInfo sys_settings_{};
        // no connected clients counter
        std::atomic_int64_t no_connected_clients_counter_ = 0;

    public:
        // adapter uid <==> D3D11Device
        std::map<uint64_t, Microsoft::WRL::ComPtr<ID3D11Device>> d3d11_devices_;
        // adapter uid <==> D3D11DeviceContext
        std::map<uint64_t, Microsoft::WRL::ComPtr<ID3D11DeviceContext>> d3d11_devices_context_;
    };

}

#endif //GAMMARAY_GR_PLUGIN_INTERFACE_H

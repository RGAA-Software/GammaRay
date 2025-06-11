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

        virtual void OnNewClientIn();

        // widget
        QWidget* GetRootWidget();
        bool eventFilter(QObject *watched, QEvent *event) override;
        void ShowRootWidget();
        void HideRootWidget();

        void AttachNetPlugin(const std::string& id, GrNetPlugin* plugin);
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
        void DispatchAllStreamMessage(const std::string& msg, bool run_through = false);

        // Serialized proto message from Renderer
        // to a specific stream
        // !! Call this function in a NON-NET-PLUGIN !!
        void DispatchTargetStreamMessage(const std::string& stream_id, const std::string& msg, bool run_through = false);

        // Serialized proto message from Renderer
        // to file transfer
        // !! Call this function in a NON-NET-PLUGIN !!
        void DispatchTargetFileTransferMessage(const std::string& stream_id, const std::string& msg, bool run_through = false);

        // messages from remote
        virtual void OnMessage(const std::shared_ptr<Message>& msg);
        // msg: Parsed messages
        virtual void OnMessageRaw(const std::any& msg);

        std::map<std::string, GrNetPlugin*> GetNetPlugins();
        int64_t GetQueuingMediaMsgCountInNetPlugins();
        int64_t GetQueuingFtMsgCountInNetPlugins();

        // app settings
        virtual void OnSyncSystemSettings(const GrPluginSettingsInfo& settings);

        // app events
        virtual void DispatchAppEvent(const std::shared_ptr<AppBaseEvent>& event) {};

        GrPluginSettingsInfo GetSystemSettings();

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
        std::string capture_audio_device_id_;
        // active net plugins...
        std::map<std::string, GrNetPlugin*> net_plugins_;
        // settings
        GrPluginSettingsInfo sys_settings_{};

    public:
        // adapter uid <==> D3D11Device
        std::map<uint64_t, Microsoft::WRL::ComPtr<ID3D11Device>> d3d11_devices_;
        // adapter uid <==> D3D11DeviceContext
        std::map<uint64_t, Microsoft::WRL::ComPtr<ID3D11DeviceContext>> d3d11_devices_context_;
    };

}

#endif //GAMMARAY_GR_PLUGIN_INTERFACE_H

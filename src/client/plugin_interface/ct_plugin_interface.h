//
// Created by RGAA on 22/05/2025.
//

#ifndef GAMMARAY_CT_PLUGIN_INTERFACE_H
#define GAMMARAY_CT_PLUGIN_INTERFACE_H

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
#include "ct_plugin_settings.h"

namespace tc
{

    class Message;
    class ClientPluginBaseEvent;
    class ClientPluginContext;
    class ClientAppBaseEvent;

    // param
    class ClientPluginParam {
    public:
        std::map<std::string, std::any> cluster_;
    };

    // plugin type
    enum class ClientPluginType {
        kStream,
        kEncoder,
        kNet,
        kUtil,
    };

    // callback
    using ClientPluginEventCallback = std::function<void(const std::shared_ptr<ClientPluginBaseEvent>&)>;

    // interface
    class ClientPluginInterface : public QObject {
    public:
        ClientPluginInterface() = default;
        ~ClientPluginInterface() override = default;

        ClientPluginInterface(const ClientPluginInterface&) = delete;
        ClientPluginInterface& operator=(const ClientPluginInterface&) = delete;

        std::shared_ptr<ClientPluginContext> GetPluginContext();

        // info
        virtual std::string GetPluginId() = 0;
        virtual std::string GetPluginName();
        virtual std::string GetPluginAuthor();
        virtual std::string GetPluginDescription();
        virtual ClientPluginType GetPluginType();

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
        virtual bool OnCreate(const ClientPluginParam& param);
        virtual bool OnResume();
        virtual bool OnStop();
        virtual bool OnDestroy();

        // task
        void PostWorkTask(std::function<void()>&& task);
        void PostUITask(std::function<void()>&& task);
        void PostUIDelayTask(int ms, std::function<void()>&& task);

        // event
        void RegisterEventCallback(const ClientPluginEventCallback& cbk);
        void CallbackEvent(const std::shared_ptr<ClientPluginBaseEvent>& event); // dll -> exe
        void CallbackEventDirectly(const std::shared_ptr<ClientPluginBaseEvent>& event);

        virtual void On1Second();

        // widget
        virtual QWidget* GetRootWidget();
        virtual void ShowRootWidget();
        virtual void HideRootWidget();
        bool eventFilter(QObject *watched, QEvent *event) override;

        // messages from remote
        virtual void OnMessage(std::shared_ptr<Message> msg);
        // msg: Parsed messages
        virtual void OnMessageRaw(const std::any& msg);

        // app events
        virtual void DispatchAppEvent(const std::shared_ptr<ClientAppBaseEvent>& event) {};  // exe -> dll

        // exe -> dlls
        virtual void SyncClientPluginSettings(const ClientPluginSettings& st);

        // plugin settings
        ClientPluginSettings GetPluginSettings();

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
        std::shared_ptr<ClientPluginContext> plugin_context_ = nullptr;
        std::atomic_bool stopped_ = false;
        std::atomic_bool destroyed_ = false;
        ClientPluginParam param_;
        ClientPluginEventCallback event_cbk_ = nullptr;
        std::string plugin_file_name_;
        ClientPluginType plugin_type_ = ClientPluginType::kUtil;
        QWidget* root_widget_ = nullptr;
        std::string plugin_author_;
        std::string plugin_desc_;
        std::string plugin_version_name_;
        int64_t plugin_version_code_;
        bool plugin_enabled_ = true;
        std::string base_path_;
        std::string capture_audio_device_id_;
        std::string screen_recording_path_;
        ClientPluginSettings plugin_settings_;
    };


}



#endif //GAMMARAY_CT_PLUGIN_INTERFACE_H

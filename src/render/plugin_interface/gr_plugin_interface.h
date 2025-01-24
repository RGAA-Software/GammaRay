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

namespace tc
{

    class Data;
    class Image;
    class GrPluginBaseEvent;
    class GrPluginContext;

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
        virtual ~GrPluginInterface() override = default;

        GrPluginInterface(const GrPluginInterface&) = delete;
        GrPluginInterface& operator=(const GrPluginInterface&) = delete;

        // info
        virtual std::string GetPluginId() = 0;
        virtual std::string GetPluginName();
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
        void PostWorkThread(std::function<void()>&& task);

        // event
        void RegisterEventCallback(const GrPluginEventCallback& cbk);
        void CallbackEvent(const std::shared_ptr<GrPluginBaseEvent>& event);
        void CallbackEventDirectly(const std::shared_ptr<GrPluginBaseEvent>& event);

        virtual void On1Second();

        // key frame
        virtual void InsertIdr();

        virtual void OnCommand(const std::string& command);

        // widget
        QWidget* GetRootWidget();
        bool eventFilter(QObject *watched, QEvent *event) override;
        void ShowRootWidget();
        void HideRootWidget();

    protected:

        bool HasParam(const std::string& k) {
            return param_.cluster_.count(k) > 0;
        }

        template<typename T>
        T GetParam(const std::string& k) {
            if (param_.cluster_.count(k) > 0) {
                return std::any_cast<T>(param_.cluster_[k]);
            }
            return T{};
        }
        std::string GetStringParam(const std::string& k) { return GetParam<std::string>(k); }
        int64_t GetIntParam(const std::string& k) { return GetParam<int64_t>(k); }
        bool GetBoolParam(const std::string& k) {return GetParam<bool>(k); }
        double GetDoubleParam(const std::string& k) { return GetParam<double>(k); }

    protected:
        std::shared_ptr<GrPluginContext> plugin_context_ = nullptr;
        std::atomic_bool enabled_ = false;
        std::atomic_bool stopped_ = false;
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
        std::string capture_monitor_name_;
        std::string capture_audio_device_id_;

    public:
        Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device_ = nullptr;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_device_context_ = nullptr;
    };

}

#endif //GAMMARAY_GR_PLUGIN_INTERFACE_H

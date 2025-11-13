//
// Created by RGAA on 22/05/2025.
//

#ifndef GAMMARAY_CT_SKIN_INTERFACE_H
#define GAMMARAY_CT_SKIN_INTERFACE_H

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
#include "skin_settings.h"

namespace tc
{

    class SkinContext;

    class SkinParam {
    public:
        std::map<std::string, std::any> cluster_;
    };

    // interface
    class SkinInterface : public QObject {
    public:
        SkinInterface() = default;
        ~SkinInterface() override = default;

        SkinInterface(const SkinInterface&) = delete;
        SkinInterface& operator=(const SkinInterface&) = delete;

        std::shared_ptr<SkinContext> GetPluginContext();

        // info
        virtual std::string GetSkinName() = 0;

        // version
        virtual std::string GetVersionName();
        virtual uint32_t GetVersionCode();

        // lifecycle
        virtual bool OnCreate(const SkinParam& param);
        virtual bool OnResume();
        virtual bool OnStop();
        virtual bool OnDestroy();

        virtual void On1Second();

        // widget
        virtual QWidget* GetRootWidget();
        virtual void ShowRootWidget();
        virtual void HideRootWidget();
        bool eventFilter(QObject *watched, QEvent *event) override;

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
        std::shared_ptr<SkinContext> plugin_context_ = nullptr;
        std::atomic_bool stopped_ = false;
        std::atomic_bool destroyed_ = false;
        SkinParam param_;
        std::string plugin_file_name_;
        QWidget* root_widget_ = nullptr;
        std::string plugin_author_;
        std::string plugin_desc_;
        std::string plugin_version_name_;
        int64_t plugin_version_code_{};
        bool plugin_enabled_ = true;
        std::string base_path_;
        std::string capture_audio_device_id_;
        std::string screen_recording_path_;
        SkinSettings plugin_settings_;
    };


}



#endif //GAMMARAY_CT_PLUGIN_INTERFACE_H

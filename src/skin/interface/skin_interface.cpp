//
// Created by RGAA on 22/05/2025.
//

#include "skin_interface.h"
#include "skin_context.h"
#include "tc_common_new/image.h"
#include "tc_common_new/data.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "snowflake/snowflake.h"
#include <QtCore/QTimer>
#include <QtCore/QEvent>

namespace tc
{

    std::shared_ptr<SkinContext> SkinInterface::GetPluginContext() {
        return plugin_context_;
    }

    std::string SkinInterface::GetVersionName() {
        return "1.0.0";
    }

    uint32_t SkinInterface::GetVersionCode() {
        return 1;
    }

    bool SkinInterface::OnCreate(const SkinParam& param) {
        SnowflakeId::initialize(0, 105);
        this->param_ = param;
        if (param.cluster_.contains("name")) {
            auto n = param.cluster_.at("name");
            plugin_file_name_ = std::any_cast<std::string>(n);
        }

        if (param.cluster_.contains("base_path")) {
            base_path_ = std::any_cast<std::string>(param.cluster_.at("base_path"));
        }
        plugin_context_ = std::make_shared<SkinContext>(GetSkinName());

        Logger::InitLog(base_path_ + "/gr_logs/skin_" + plugin_file_name_+".log", true);
        LOGI("{} OnCreate", GetSkinName());

        capture_audio_device_id_ = GetConfigParam<std::string>("capture_audio_device_id");

        screen_recording_path_ = GetConfigParam<std::string>("screen_recording_path");

        plugin_settings_.clipboard_enabled_ = GetConfigBoolParam("clipboard_enabled");
        plugin_settings_.device_id_ = GetConfigStringParam("device_id");
        plugin_settings_.stream_id_ = GetConfigStringParam("stream_id");
        plugin_settings_.language_ = (int)GetConfigIntParam("language");
        plugin_settings_.stream_name_ = GetConfigStringParam("stream_name");
        plugin_settings_.display_name_ = GetConfigStringParam("display_name");
        plugin_settings_.display_remote_name_ = GetConfigStringParam("display_remote_name");

        LOGI("plugin settings clipboard enabled: {}", plugin_settings_.clipboard_enabled_);
        LOGI("plugin settings device id: {}", plugin_settings_.device_id_);
        LOGI("plugin settings stream id: {}", plugin_settings_.stream_id_);

        // print params
        LOGI("Input params size : {}", param.cluster_.size());
        for (const auto& [key, value]: param.cluster_) {
            if (value.type() == typeid(std::string)) {
                LOGI(" * {} => {}", key, std::any_cast<std::string>(value));
            }
            else if (value.type() == typeid(int64_t)) {
                LOGI(" * {} => {}", key, std::any_cast<int64_t>(value));
            }
            else if (value.type() == typeid(double)) {
                LOGI(" * {} => {}", key, std::any_cast<double>(value));
            }
            else if (value.type() == typeid(bool)) {
                LOGI(" * {} => {}", key, std::any_cast<bool>(value));
            }

            // parse
            if (key == "author") {
                plugin_author_ = std::any_cast<std::string>(value);
            }
            else if (key == "description") {
                plugin_desc_ = std::any_cast<std::string>(value);
            }
            else if (key == "version_name") {
                plugin_version_name_ = std::any_cast<std::string>(value);
            }
            else if (key == "version_code") {
                plugin_version_code_ = std::any_cast<int64_t>(value);
            }
            else if (key == "enabled") {
                plugin_enabled_ = std::any_cast<bool>(value);
            }
        }

        root_widget_ = new QWidget();
        root_widget_->resize(960, 540);
        root_widget_->hide();
        root_widget_->installEventFilter(this);

        return true;
    }

    bool SkinInterface::OnResume() {
        this->stopped_ = false;
        return true;
    }

    bool SkinInterface::OnStop() {
        this->stopped_ = true;
        return true;
    }

    bool SkinInterface::OnDestroy() {
        plugin_context_->OnDestroy();
        destroyed_ = true;
        return true;
    }

    void SkinInterface::On1Second() {

    }

    QWidget* SkinInterface::GetRootWidget() {
        return root_widget_;
    }

    bool SkinInterface::eventFilter(QObject *watched, QEvent *event) {
        if (watched == root_widget_) {
            if (event->type() == QEvent::Type::Close) {
                LOGI("Event: {}", (int) event->type());
                event->ignore();
                ((QWidget*)watched)->hide();
                return true;
            }
        }
        return QObject::eventFilter(watched, event);
    }

    void SkinInterface::ShowRootWidget() {
        root_widget_->show();
    }

    void SkinInterface::HideRootWidget() {
        root_widget_->hide();
    }

}

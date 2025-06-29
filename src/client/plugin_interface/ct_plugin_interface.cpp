//
// Created by RGAA on 22/05/2025.
//

#include "ct_plugin_interface.h"
#include "ct_plugin_context.h"
#include "tc_common_new/image.h"
#include "tc_common_new/data.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include <QtCore/QTimer>
#include <QtCore/QEvent>

namespace tc
{

    std::shared_ptr<ClientPluginContext> ClientPluginInterface::GetPluginContext() {
        return plugin_context_;
    }

    std::string ClientPluginInterface::GetPluginName() {
        return "dummy";
    }

    std::string ClientPluginInterface::GetPluginAuthor() {
        return plugin_author_;
    }

    std::string ClientPluginInterface::GetPluginDescription() {
        return "plugin description";
    }

    ClientPluginType ClientPluginInterface::GetPluginType() {
        return plugin_type_;
    }

    std::string ClientPluginInterface::GetVersionName() {
        return "1.0.0";
    }

    uint32_t ClientPluginInterface::GetVersionCode() {
        return 1;
    }

    bool ClientPluginInterface::IsPluginEnabled() {
        return plugin_enabled_;
    }

    void ClientPluginInterface::EnablePlugin() {
        plugin_enabled_ = true;
    }

    void ClientPluginInterface::DisablePlugin() {
        plugin_enabled_ = false;
    }

    bool ClientPluginInterface::IsWorking() {
        return false;
    }

    bool ClientPluginInterface::OnCreate(const ClientPluginParam& param) {
        this->param_ = param;
        if (param.cluster_.contains("name")) {
            auto n = param.cluster_.at("name");
            plugin_file_name_ = std::any_cast<std::string>(n);
        }

        if (param.cluster_.contains("base_path")) {
            base_path_ = std::any_cast<std::string>(param.cluster_.at("base_path"));
        }
        plugin_context_ = std::make_shared<ClientPluginContext>(GetPluginName());

        Logger::InitLog(base_path_ + "/gr_logs/ct_" + plugin_file_name_+".log", true);
        LOGI("{} OnCreate", GetPluginName());

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

    bool ClientPluginInterface::OnResume() {
        this->stopped_ = false;
        return true;
    }

    bool ClientPluginInterface::OnStop() {
        this->stopped_ = true;
        return true;
    }

    bool ClientPluginInterface::OnDestroy() {
        plugin_context_->OnDestroy();
        destroyed_ = true;
        return true;
    }

    void ClientPluginInterface::PostWorkTask(std::function<void()>&& task) {
        if (plugin_context_ && !stopped_) {
            plugin_context_->PostWorkTask(std::move(task));
        }
    }

    void ClientPluginInterface::PostUITask(std::function<void()>&& task) {
        QMetaObject::invokeMethod(this, [=, this]() {
            task();
        });
    }

    void ClientPluginInterface::PostUIDelayTask(int ms, std::function<void()>&& task) {
        this->PostUITask([ms, t = std::move(task)]() {
            QTimer::singleShot(ms, [=]() {
                t();
            });
        });
    }

    void ClientPluginInterface::RegisterEventCallback(const ClientPluginEventCallback& cbk) {
        event_cbk_ = cbk;
    }

    void ClientPluginInterface::CallbackEvent(const std::shared_ptr<ClientPluginBaseEvent>& event) {
        if (!event_cbk_) {
            return;
        }
        PostWorkTask([=, this]() {
            event_cbk_(event);
        });
    }

    void ClientPluginInterface::CallbackEventDirectly(const std::shared_ptr<ClientPluginBaseEvent>& event) {
        if (event_cbk_) {
            event_cbk_(event);
        }
    }

    void ClientPluginInterface::On1Second() {

    }

    QWidget* ClientPluginInterface::GetRootWidget() {
        return root_widget_;
    }

    bool ClientPluginInterface::eventFilter(QObject *watched, QEvent *event) {
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

    void ClientPluginInterface::ShowRootWidget() {
        root_widget_->show();
    }

    void ClientPluginInterface::HideRootWidget() {
        root_widget_->hide();
    }

    void ClientPluginInterface::OnMessage(std::shared_ptr<Message> msg) {

    }

    void ClientPluginInterface::OnMessageRaw(const std::any& msg) {

    }

    void ClientPluginInterface::SyncClientPluginSettings(const ClientPluginSettings& st) {
        plugin_settings_.clipboard_enabled_ = st.clipboard_enabled_;
    }

    ClientPluginSettings ClientPluginInterface::GetPluginSettings() {
        return plugin_settings_;
    }

    bool ClientPluginInterface::HasProcessingTasks() {
        return false;
    }

}

//
// Created by RGAA on 15/11/2024.
//

#include "gr_plugin_interface.h"
#include "tc_common_new/image.h"
#include "tc_common_new/data.h"
#include "tc_common_new/thread.h"
#include "gr_plugin_events.h"
#include "tc_common_new/log.h"
#include "gr_plugin_context.h"

#include <QtCore/QEvent>

namespace tc
{

    std::string GrPluginInterface::GetPluginName() {
        return "dummy";
    }

    std::string GrPluginInterface::GetPluginDescription() {
        return "plugin description";
    }

    GrPluginType GrPluginInterface::GetPluginType() {
        return plugin_type_;
    }

    bool GrPluginInterface::IsStreamPlugin() {
        return plugin_type_ == GrPluginType::kStream;
    }

    std::string GrPluginInterface::GetVersionName() {
        return "1.0.0";
    }

    uint32_t GrPluginInterface::GetVersionCode() {
        return 1;
    }

    bool GrPluginInterface::IsPluginEnabled() {
        return enabled_;
    }

    void GrPluginInterface::EnablePlugin() {
        enabled_ = true;
    }

    void GrPluginInterface::DisablePlugin() {
        enabled_ = false;
    }

    bool GrPluginInterface::IsWorking() {
        return false;
    }

    bool GrPluginInterface::OnCreate(const GrPluginParam& param) {
        this->param_ = param;
        if (param.cluster_.contains("name")) {
            auto n = param.cluster_.at("name");
            plugin_file_name_ = std::any_cast<std::string>(n);
        }

        std::string base_path;
        if (param.cluster_.contains("base_path")) {
            base_path = std::any_cast<std::string>(param.cluster_.at("base_path"));
        }
        plugin_context_ = std::make_shared<GrPluginContext>(GetPluginName());

        Logger::InitLog(base_path + "/gr_logs/" + plugin_file_name_+".log", true);
        LOGI("{} OnCreate", GetPluginName());

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

        this->enabled_ = plugin_enabled_;

        root_widget_ = new QWidget();
        root_widget_->resize(960, 540);
        root_widget_->hide();
        root_widget_->installEventFilter(this);

        return true;
    }

    bool GrPluginInterface::OnResume() {
        this->stopped_ = false;
        return true;
    }

    bool GrPluginInterface::OnStop() {
        this->stopped_ = true;
        return true;
    }

    bool GrPluginInterface::OnDestroy() {
        plugin_context_->OnDestroy();
        return true;
    }

    void GrPluginInterface::PostWorkThread(std::function<void()>&& task) {
        if (plugin_context_ && !stopped_) {
            plugin_context_->PostWorkThread(std::move(task));
        }
    }

    void GrPluginInterface::RegisterEventCallback(const GrPluginEventCallback& cbk) {
        event_cbk_ = cbk;
    }

    void GrPluginInterface::CallbackEvent(const std::shared_ptr<GrPluginBaseEvent>& event) {
        if (!event_cbk_) {
            return;
        }
        PostWorkThread([=, this]() {
            event_cbk_(event);
        });
    }

    void GrPluginInterface::CallbackEventDirectly(const std::shared_ptr<GrPluginBaseEvent>& event) {
        if (event_cbk_) {
            event_cbk_(event);
        }
    }

    void GrPluginInterface::On1Second() {

    }

    void GrPluginInterface::InsertIdr() {
        auto event = std::make_shared<GrPluginInsertIdrEvent>();
        CallbackEvent(event);
    }

    void GrPluginInterface::OnCommand(const std::string& command) {

    }

    QWidget* GrPluginInterface::GetRootWidget() {
        return root_widget_;
    }

    bool GrPluginInterface::eventFilter(QObject *watched, QEvent *event) {
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

    void GrPluginInterface::ShowRootWidget() {
        root_widget_->show();
    }

    void GrPluginInterface::HideRootWidget() {
        root_widget_->hide();
    }

}
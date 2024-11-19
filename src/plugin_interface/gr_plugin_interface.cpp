//
// Created by RGAA on 15/11/2024.
//

#include "gr_plugin_interface.h"
#include "tc_common_new/image.h"
#include "tc_common_new/data.h"
#include "tc_common_new/thread.h"
#include "gr_plugin_events.h"
#include "tc_common_new/log.h"

#include <QtCore/QEvent>

namespace tc
{

    std::string GrPluginInterface::GetPluginName() {
        return "dummy";
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
        this->enabled_ = true;
        if (param.cluster_.contains("name")) {
            auto n = param.cluster_.at("name");
            plugin_file_name_ = std::any_cast<std::string>(n);
        }

        work_thread_ = Thread::Make(GetPluginName(), 1024);
        work_thread_->Poll();

        timer_ = std::make_shared<asio2::timer>();

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
        if (work_thread_) {
            work_thread_->Exit();
        }
        if (timer_) {
            timer_->stop_all_timers();
        }
        return true;
    }

    void GrPluginInterface::PostWorkThread(std::function<void()>&& task) {
        if (work_thread_ && !stopped_) {
            work_thread_->Post(std::move(task));
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

    void GrPluginInterface::StartTimer(int millis, std::function<void()>&& cbk) {
        if (timer_) {
            timer_->start_timer(std::to_string(millis), millis, std::move(cbk));
        }
    }

    void GrPluginInterface::On1Second() {

    }

    void GrPluginInterface::InsertIdr() {
        auto event = std::make_shared<GrPluginInsertIdrEvent>();
        CallbackEvent(event);
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
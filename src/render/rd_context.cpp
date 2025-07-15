//
// Created by RGAA on 2023/12/18.
//

#include "rd_context.h"
#include <QTimer>
#include <QApplication>
#include "tc_common_new/task_runtime.h"
#include "tc_common_new/message_notifier.h"
#include "plugins/plugin_manager.h"
#include "plugin_interface/gr_plugin_interface.h"

namespace tc
{
    std::shared_ptr<RdContext> RdContext::Make() {
        return std::make_shared<RdContext>();
    }

    RdContext::RdContext() {
        msg_notifier_ = std::make_shared<MessageNotifier>();
        task_rt_ = std::make_shared<TaskRuntime>(8);

        stream_plugin_thread_ = Thread::Make("stream plugin thread", 128);
        stream_plugin_thread_->Poll();
    }

    bool RdContext::Init() {
        return true;
    }

    std::shared_ptr<MessageNotifier> RdContext::GetMessageNotifier() {
        return msg_notifier_;
    }

    std::shared_ptr<MessageListener> RdContext::CreateMessageListener() {
        return msg_notifier_->CreateListener();
    }

    void RdContext::SetPluginManager(const std::shared_ptr<PluginManager>& pm) {
        plugin_manager_ = pm;
    }

    std::shared_ptr<PluginManager> RdContext::GetPluginManager() {
        return plugin_manager_;
    }

    void RdContext::PostTask(std::function<void()>&& task) {
        task_rt_->Post(SimpleThreadTask::Make(std::move(task)));
    }

    void RdContext::PostUITask(std::function<void()>&& task) {
        QMetaObject::invokeMethod(this, [=]() {
            task();
        });
    }

    void RdContext::PostDelayTask(std::function<void()>&& task, int delay) {
        QTimer::singleShot(delay, [t = std::move(task)]() {
            t();
        });
    }

    void RdContext::PostStreamPluginTask(std::function<void()>&& task) {
        stream_plugin_thread_->Post(std::move(task));
    }

    std::string RdContext::GetCurrentExeFolder() {
        return QCoreApplication::applicationDirPath().toStdString();
    }

    void RdContext::DispatchAppEvent2Plugins(const std::shared_ptr<AppBaseEvent>& event) {
        plugin_manager_->VisitAllPlugins([=, this](GrPluginInterface* plugin) {
            plugin->DispatchAppEvent(event);
        });
    }

}
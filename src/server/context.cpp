//
// Created by RGAA on 2023/12/18.
//

#include "context.h"
#include "tc_common_new/task_runtime.h"
#include "tc_common_new/message_notifier.h"
#include "plugins/plugin_manager.h"
#include <QApplication>

namespace tc
{
    std::shared_ptr<Context> Context::Make() {
        return std::make_shared<Context>();
    }

    Context::Context() {
        msg_notifier_ = std::make_shared<MessageNotifier>();
        asio2_pool_ = std::make_shared<asio2::iopool>();
        asio2_pool_->start();
    }

    bool Context::Init() {
        plugin_manager_ = PluginManager::Make(shared_from_this());
        plugin_manager_->LoadAllPlugins();
        plugin_manager_->RegisterPluginEventsCallback();
        plugin_manager_->DumpPluginInfo();
        return true;
    }

    std::shared_ptr<MessageNotifier> Context::GetMessageNotifier() {
        return msg_notifier_;
    }

    std::shared_ptr<MessageListener> Context::CreateMessageListener() {
        return msg_notifier_->CreateListener();
    }

    std::shared_ptr<PluginManager> Context::GetPluginManager() {
        return plugin_manager_;
    }

    void Context::PostTask(std::function<void()>&& task) {
        asio2_pool_->post(std::move(task));
    }

    std::shared_ptr<asio2::iopool> Context::GetAsio2IoPool() {
        return asio2_pool_;
    }

    std::string Context::GetCurrentExeFolder() {
        return QCoreApplication::applicationDirPath().toStdString();
    }

}
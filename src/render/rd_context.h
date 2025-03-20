//
// Created by RGAA on 2023/12/18.
//

#ifndef TC_APPLICATION_CONTEXT_H
#define TC_APPLICATION_CONTEXT_H

#include <memory>

#include "dexode/EventBus.hpp"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/thread.h"
#include "asio2/asio2.hpp"
#include <QObject>

namespace tc
{

    class PluginManager;

    class RdContext : public QObject, public std::enable_shared_from_this<RdContext> {
    public:
        static std::shared_ptr<RdContext> Make();

        RdContext();
        ~RdContext() = default;

        bool Init();
        void SetPluginManager(const std::shared_ptr<PluginManager>& pm);

        std::shared_ptr<MessageNotifier> GetMessageNotifier();
        std::shared_ptr<MessageListener> CreateMessageListener();
        std::shared_ptr<PluginManager> GetPluginManager();

        template<typename T>
        void SendAppMessage(const T& m) {
            asio2_pool_->post([=, this]() {
                if (msg_notifier_) {
                    msg_notifier_->SendAppMessage(m);
                }
            });
        }

        void PostTask(std::function<void()>&& task);
        void PostUITask(std::function<void()>&& task);
        void PostStreamPluginTask(std::function<void()>&& task);
        std::shared_ptr<asio2::iopool> GetAsio2IoPool();
        static std::string GetCurrentExeFolder();

    private:
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::shared_ptr<Thread> msg_thread_ = nullptr;
        std::shared_ptr<asio2::iopool> asio2_pool_ = nullptr;
        std::shared_ptr<PluginManager> plugin_manager_ = nullptr;
        std::shared_ptr<Thread> stream_plugin_thread_ = nullptr;
    };
}

#endif //TC_APPLICATION_CONTEXT_H

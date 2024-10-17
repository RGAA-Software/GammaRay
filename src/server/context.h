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

namespace tc
{

    class Context {
    public:
        static std::shared_ptr<Context> Make();

        Context();
        ~Context() = default;

        std::shared_ptr<MessageNotifier> GetMessageNotifier();
        std::shared_ptr<MessageListener> CreateMessageListener();

        template<typename T>
        void SendAppMessage(const T& m) {
            asio2_pool_->post([=, this]() {
                if (msg_notifier_) {
                    msg_notifier_->SendAppMessage(m);
                }
            });
        }

        void PostTask(std::function<void()>&& task);
        std::shared_ptr<asio2::iopool> GetAsio2IoPool();
        static std::string GetCurrentExeFolder();

    private:
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::shared_ptr<Thread> msg_thread_ = nullptr;
        std::shared_ptr<asio2::iopool> asio2_pool_ = nullptr;
    };
}

#endif //TC_APPLICATION_CONTEXT_H

//
// Created by RGAA on 21/10/2024.
//

#ifndef GAMMARAY_SERVICE_CONTEXT_H
#define GAMMARAY_SERVICE_CONTEXT_H

#include <memory>
#include <asio2/asio2.hpp>
#include "tc_common_new/message_notifier.h"

namespace tc
{

    class SharedPreference;

    class ServiceContext {
    public:
        ServiceContext(int port);

        void PostBgTask(std::function<void()>&& task);
        std::shared_ptr<MessageListener> CreateMessageListener();
        SharedPreference* GetSp() { return sp_; }
        int GetListeningPort() {return listening_port_;}
        std::string GetAppExeFolderPath();

        template<typename T>
        void SendAppMessage(const T& m) {
            if (msg_notifier_) {
                msg_notifier_->SendAppMessage(m);
            }
        }

    private:
        std::shared_ptr<asio2::timer> timer_ = nullptr;
        std::shared_ptr<asio2::iopool> iopool_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        SharedPreference* sp_ = nullptr;
        int listening_port_ = 0;
    };

}

#endif //GAMMARAY_SERVICE_CONTEXT_H

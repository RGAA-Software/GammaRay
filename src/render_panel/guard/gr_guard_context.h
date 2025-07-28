//
// Created by RGAA on 28/07/2025.
//

#ifndef GAMMARAYPREMIUM_GR_GUARD_CONTEXT_H
#define GAMMARAYPREMIUM_GR_GUARD_CONTEXT_H

#include <memory>
#include "tc_common_new/message_notifier.h"

namespace asio2
{
    class timer;
}

namespace tc
{

    class GrGuardContext {
    public:
        GrGuardContext();

        void StartTimers();
        std::shared_ptr<MessageNotifier> GetMessageNotifier();
        std::shared_ptr<MessageListener> ObtainMessageListener();

        template<typename T>
        void SendAppMessage(const T& m) {
            if (msg_notifier_) {
                msg_notifier_->SendAppMessage(m);
            }
        }

    private:
        std::shared_ptr<asio2::timer> timer_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
    };

}

#endif //GAMMARAYPREMIUM_GR_GUARD_CONTEXT_H

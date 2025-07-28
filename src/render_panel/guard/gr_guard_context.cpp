//
// Created by RGAA on 28/07/2025.
//

#include "gr_guard_context.h"
#include <asio2/asio2.hpp>
#include "gr_guard_messages.h"

namespace tc
{

    GrGuardContext::GrGuardContext() {
        msg_notifier_ = std::make_shared<MessageNotifier>();
        StartTimers();
    }

    std::shared_ptr<MessageNotifier> GrGuardContext::GetMessageNotifier() {
        return msg_notifier_;
    }

    std::shared_ptr<MessageListener> GrGuardContext::ObtainMessageListener() {
        return msg_notifier_->CreateListener();
    }


    void GrGuardContext::StartTimers() {
        timer_ = std::make_shared<asio2::timer>();
        timer_->start_timer(1, 100, [=, this]() {
            this->SendAppMessage(MsgGrGuardTimer100{});
        });

        timer_->start_timer(2, 1000, [=, this]() {
            this->SendAppMessage(MsgGrGuardTimer1S{});
        });

        timer_->start_timer(5, 5000, [=, this]() {
            this->SendAppMessage(MsgGrGuardTimer5S{});
        });
    }
}
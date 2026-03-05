//
// Created by RGAA on 2024-02-05.
//

#include "app_timer.h"
#include "rd_context.h"
#include "app/app_messages.h"
#include "tc_common_new/log.h"
#include "tc_common_new/fps_stat.h"

#include <vector>
#include <format>
#include <ranges>

namespace tc
{

    AppTimer::AppTimer(const std::shared_ptr<RdContext>& ctx) {
        context_ = ctx;
    }

    void AppTimer::StartTimers() {
        const auto durations = std::vector {
            kTimerDuration1000,
            kTimerDuration2000,
            kTimerDuration5000,
            kTimerDuration10S,
            kTimerDuration20S,
            kTimerDuration30S,
            kTimerDuration1Minute,
            kTimerDuration500,
            kTimerDuration100,
            kTimerDuration16,
        };
        for (const auto& duration : durations) {
            auto timer = std::make_shared<QTimer>();
            timers_.insert({duration, timer});
            connect(timer.get(), &QTimer::timeout, this, [=, this]() {
                NotifyTimeout(duration);
            });
            timer->start((int)duration);
        }
    }

    void AppTimer::StopTimers() {
        for (const auto &timer: timers_ | std::views::values) {
            timer->stop();
        }
    }

    void AppTimer::NotifyTimeout(const AppTimerDuration duration) const {
        if (duration == kTimerDuration1000) {
            context_->SendAppMessage(MsgTimer1000{});
        }
        else if (duration == kTimerDuration2000) {
            context_->SendAppMessage(MsgTimer2000{});
        }
        else if (duration == kTimerDuration5000) {
            context_->SendAppMessage(MsgTimer5000{});
        }
        else if (duration == kTimerDuration10S) {
            context_->SendAppMessage(MsgTimer10S{});
        }
        else if (duration == kTimerDuration20S) {
            context_->SendAppMessage(MsgTimer20S{});
        }
        else if (duration == kTimerDuration30S) {
            context_->SendAppMessage(MsgTimer30S{});
        }
        else if (duration == kTimerDuration500) {
            context_->SendAppMessage(MsgTimer500{});
        }
        else if (duration == kTimerDuration100) {
            context_->SendAppMessage(MsgTimer100{});
        }
        else if (duration == kTimerDuration16) {
            context_->SendAppMessage(MsgTimer16{});
        }
        else if (duration == kTimerDuration1Minute) {
            context_->SendAppMessage(MsgTimer1Minute{});
        }
    }

}

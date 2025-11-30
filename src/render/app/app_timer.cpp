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

namespace tc
{

    static FpsStat g_timer_16_fps;

    AppTimer::AppTimer(const std::shared_ptr<RdContext>& ctx) {
        context_ = ctx;
    }

    void AppTimer::StartTimers() {
        auto durations = std::vector<AppTimerDuration>{
            kTimerDuration1000,
            kTimerDuration2000,
            kTimerDuration5000,
            kTimerDuration10S,
            kTimerDuration20S,
            kTimerDuration30S,
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
        for (const auto& [duration, timer] : timers_) {
            timer->stop();
        }
    }

    void AppTimer::NotifyTimeout(AppTimerDuration duration) {
        if (duration == AppTimerDuration::kTimerDuration1000) {
            context_->SendAppMessage(MsgTimer1000{});
            //LOGI("16ms timer fps: {}", g_timer_16_fps.value());
        }
        else if (duration == AppTimerDuration::kTimerDuration2000) {
            context_->SendAppMessage(MsgTimer2000{});
        }
        else if (duration == AppTimerDuration::kTimerDuration5000) {
            context_->SendAppMessage(MsgTimer5000{});
        }
        else if (duration == AppTimerDuration::kTimerDuration10S) {
            context_->SendAppMessage(MsgTimer10S{});
        }
        else if (duration == AppTimerDuration::kTimerDuration20S) {
            context_->SendAppMessage(MsgTimer20S{});
        }
        else if (duration == AppTimerDuration::kTimerDuration30S) {
            context_->SendAppMessage(MsgTimer30S{});
        }
        else if (duration == AppTimerDuration::kTimerDuration500) {
            context_->SendAppMessage(MsgTimer500{});
        }
        else if (duration == AppTimerDuration::kTimerDuration100) {
            context_->SendAppMessage(MsgTimer100{});
        }
        else if (duration == AppTimerDuration::kTimerDuration16) {
            context_->SendAppMessage(MsgTimer16{});
            //g_timer_16_fps.Tick();
        }
    }

}
//
// Created by RGAA on 2024-02-05.
//

#ifndef TC_APPLICATION_APP_TIMER_H
#define TC_APPLICATION_APP_TIMER_H

#include <memory>
#include <functional>
#include <unordered_map>
#include <asio2/asio2.hpp>

namespace tc
{

    class RdContext;

    enum AppTimerDuration {
        kTimerDuration1000 = 1000,
        kTimerDuration2000 = 2000,
        kTimerDuration5000 = 5000,
        kTimerDuration100 = 100,
        kTimerDuration16 = 16,
    };

    class AppTimer {
    public:

        explicit AppTimer(const std::shared_ptr<RdContext>& ctx);
        void StartTimers();

    private:
        void NotifyTimeout(AppTimerDuration duration);

    private:
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<asio2::timer> timer_ = nullptr;
    };

}

#endif //TC_APPLICATION_APP_TIMER_H

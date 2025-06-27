//
// Created by RGAA on 19/11/2024.
//

#include "gr_plugin_context.h"
#include "tc_common_new/thread.h"
#include "asio2/asio2.hpp"

namespace tc
{

    GrPluginContext::GrPluginContext(const std::string& plugin_name) {
        work_thread_ = Thread::Make(plugin_name, 1024 * 1024 * 10);
        work_thread_->Poll();

        timer_ = std::make_shared<asio2::timer>();
    }

    void GrPluginContext::OnDestroy() {
        if (work_thread_) {
            work_thread_->Exit();
        }
        if (timer_) {
            timer_->stop_all_timers();
        }
    }

    void GrPluginContext::PostWorkTask(std::function<void()>&& task) {
        if (work_thread_) {
            work_thread_->Post(std::move(task));
        }
    }

    void GrPluginContext::PostUITask(std::function<void()>&& task) {
        QMetaObject::invokeMethod(this, [t = std::move(task)]() {
            t();
        });
    }

    void GrPluginContext::StartTimer(int millis, std::function<void()>&& cbk) {
        if (timer_) {
            timer_->start_timer(std::to_string(millis), millis, std::move(cbk));
        }
    }

}

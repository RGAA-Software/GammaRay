//
// Created by hy on 2024/3/26.
//

#include "server_monitor.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"

namespace tc
{

    std::shared_ptr<ServerMonitor> ServerMonitor::Make(const std::shared_ptr<Application>& app) {
        return std::make_shared<ServerMonitor>(app);
    }

    ServerMonitor::ServerMonitor(const std::shared_ptr<Application>& ctx) {
        this->app_ = ctx;
    }

    void ServerMonitor::Start() {
        monitor_thread_ = std::make_shared<Thread>([=, this]() {
            while (!exit_) {
                LOGI("server checking ...");
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }, "", false);
    }

    void ServerMonitor::Exit() {
        exit_ = true;
        if (monitor_thread_->IsJoinable()) {
            monitor_thread_->Join();
        }
    }

}
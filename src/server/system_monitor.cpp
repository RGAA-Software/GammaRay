//
// Created by hy on 2024/3/26.
//

#include "system_monitor.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "tc_controller/vigem_driver_manager.h"
#include "tc_controller/vigem/sdk/ViGEm/Client.h"

namespace tc
{

    std::shared_ptr<SystemMonitor> SystemMonitor::Make(const std::shared_ptr<Application>& app) {
        return std::make_shared<SystemMonitor>(app);
    }

    SystemMonitor::SystemMonitor(const std::shared_ptr<Application>& ctx) {
        this->app_ = ctx;
    }

    void SystemMonitor::Start() {
        vigem_driver_manager_ = VigemDriverManager::Make();

        monitor_thread_ = std::make_shared<Thread>([=, this]() {
            while (!exit_) {
                // 1. checking vigem driver is installed or not
                bool detect_ok = false;
                if (!vigem_driver_manager_->IsVigemDriverInstalled()) {
                    detect_ok = vigem_driver_manager_->Detect();
                }
                // 2. driver is not exist, to install
                if (vigem_driver_manager_->IsVigemDriverInstalled() && detect_ok) {
                    // installed & running
                    //LOGI("Installed and running");
                } else {
                    // 3. driver seems already exists, try to connect
                    if (!connect_vigem_success_) {
                        connect_vigem_success_ = vigem_driver_manager_->TryConnect();
                    }
                    // 4. connect failed, to install
                    if (!connect_vigem_success_) {
                        LOGI("connect failed.");
                        vigem_driver_manager_->InstallViGem();
                    } else {
                        LOGI("connect to vigem success.");
                    }
                }

                LOGI("system checking ...");
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }, "", false);
    }

    void SystemMonitor::Exit() {
        exit_ = true;
        if (monitor_thread_->IsJoinable()) {
            monitor_thread_->Join();
        }
    }

}
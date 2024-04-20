//
// Created by RGAA on 2024/3/26.
//

#ifndef TC_APPLICATION_SYSTEM_MONITOR_H
#define TC_APPLICATION_SYSTEM_MONITOR_H

#include <memory>
#include <functional>

namespace tc
{

    class Thread;
    class Application;
    class VigemDriverManager;

    class ServerMonitor {
    public:

        static std::shared_ptr<ServerMonitor> Make(const std::shared_ptr<Application>& app);

        explicit ServerMonitor(const std::shared_ptr<Application>& app);
        void Start();
        void Exit();

    private:
        std::shared_ptr<Application> app_ = nullptr;
        std::shared_ptr<Thread> monitor_thread_ = nullptr;
        bool exit_ = false;
    };

}

#endif //TC_APPLICATION_SYSTEM_MONITOR_H

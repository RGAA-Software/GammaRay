//
// Created by hy on 2024/3/26.
//

#ifndef TC_APPLICATION_SYSTEM_MONITOR_H
#define TC_APPLICATION_SYSTEM_MONITOR_H

#include <memory>
#include <functional>
#include <string>
#include "tc_common_new/response.h"

namespace tc
{

    class Thread;
    class GrContext;
    class VigemDriverManager;
    class MessageListener;

    class SystemMonitor {
    public:

        static std::shared_ptr<SystemMonitor> Make(const std::shared_ptr<GrContext>& ctx);

        explicit SystemMonitor(const std::shared_ptr<GrContext>& ctx);
        void Start();
        void Exit();

    private:
        static bool CheckViGEmDriver();
        bool TryConnectViGEmDriver();
        static bool GetFileVersion(const std::wstring& filePath, unsigned long& major, unsigned long& minor);
        static void InstallViGem(bool silent);
        void NotifyViGEnState(bool ok);
        void RegisterMessageListener();
        Response<bool, bool> CheckServerAlive();
        void StartServer();

    private:
        std::shared_ptr<GrContext> ctx_ = nullptr;
        std::shared_ptr<Thread> monitor_thread_ = nullptr;
        bool exit_ = false;

        std::shared_ptr<VigemDriverManager> vigem_driver_manager_ = nullptr;
        bool connect_vigem_success_ = false;

        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };

}

#endif //TC_APPLICATION_SYSTEM_MONITOR_H

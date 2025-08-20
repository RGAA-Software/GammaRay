//
// Created by RGAA on 2024/3/26.
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
    class GrApplication;
    class VigemDriverManager;
    class MessageListener;
    class ServiceManager;
    class GrSettings;
    class SpvrManager;

    class GrSystemMonitor {
    public:

        static std::shared_ptr<GrSystemMonitor> Make(const std::shared_ptr<GrApplication>& app);

        explicit GrSystemMonitor(const std::shared_ptr<GrApplication>& app);
        void Start();
        void Exit();

    private:
        static bool CheckViGEmDriver();
        bool TryConnectViGEmDriver();
        static bool GetFileVersion(const std::wstring& filePath, unsigned long& major, unsigned long& minor);
        static void InstallViGem(bool silent);
        void NotifyViGEnState(bool ok);
        void RegisterMessageListener();
        Response<bool, bool> CheckRenderAlive();
        void CheckServiceAlive();
        void StartServer();
        // bool VerifyOnlineServers();
        // void CheckOnlineServers();
        void CheckThisDeviceInfo();

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<Thread> monitor_thread_ = nullptr;
        bool exit_ = false;

        std::shared_ptr<VigemDriverManager> vigem_driver_manager_ = nullptr;
        bool connect_vigem_success_ = false;

        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<ServiceManager> service_manager_ = nullptr;
    };

}

#endif //TC_APPLICATION_SYSTEM_MONITOR_H

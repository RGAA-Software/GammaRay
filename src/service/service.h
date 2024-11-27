//
// Created by RGAA on 21/10/2024.
//

#ifndef GAMMARAY_SERVICE_H
#define GAMMARAY_SERVICE_H

#include <Windows.h>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <functional>

namespace tc
{

    using SrvTask = std::function<void()>;

    class ServiceContext;

    class GrService : public std::enable_shared_from_this<GrService> {
    public:
        explicit GrService(const std::shared_ptr<ServiceContext>& ctx);

        void Run(DWORD argc, LPWSTR* argv, SERVICE_STATUS_HANDLE handle);

        void OnCtrlStop();
        void OnCtrlContinue();
        void OnCtrlPause();
        void OnCtrlInterrogate();

        void OnConsoleConnect(int id);
        void OnConsoleDisConnect(int id);
        void OnSessionLogon(int id);
        void OnSessionLogoff(int id);
        void OnSessionLock(int id);
        void OnSessionUnlock(int id);

        void TaskThread();
        void PostTask(SrvTask&& task);
        void SetStatus(DWORD dwState, DWORD dwErrCode = NO_ERROR, DWORD dwWait = 0);
        // stop all apps
        void StopAll();
        void SimulateCtrlAltDelete();

    private:
        std::shared_ptr<ServiceContext> context_ = nullptr;
        SERVICE_STATUS service_status_ = { 0 };
        SERVICE_STATUS_HANDLE status_handle_ = nullptr;

        std::mutex cv_mtx_;
        std::condition_variable cv_;
        std::queue<SrvTask> tasks_;
        std::atomic_bool exit_ = false;
        std::thread task_thread_;
    };

}

#endif //GAMMARAY_SERVICE_H

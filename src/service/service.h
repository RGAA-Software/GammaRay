//
// Created by RGAA on 21/10/2024.
//

#ifndef GAMMARAY_SERVICE_H
#define GAMMARAY_SERVICE_H

#include <memory>
#include <Windows.h>

namespace tc
{

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

    private:
        std::shared_ptr<ServiceContext> context_ = nullptr;

    };

}

#endif //GAMMARAY_SERVICE_H

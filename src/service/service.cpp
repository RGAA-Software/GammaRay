//
// Created by RGAA on 21/10/2024.
//

#include "service.h"
#include "service_context.h"

namespace tc
{

    GrService::GrService(const std::shared_ptr<ServiceContext>& ctx) {
        context_ = ctx;
    }

    void GrService::Run(DWORD dwArgc, LPWSTR* lpszArgv, SERVICE_STATUS_HANDLE handle) {

    }

    void GrService::OnCtrlStop() {

    }

    void GrService::OnCtrlContinue() {

    }

    void GrService::OnCtrlPause() {

    }

    void GrService::OnCtrlInterrogate() {

    }

    void GrService::OnConsoleConnect(int id) {

    }

    void GrService::OnConsoleDisConnect(int id) {

    }

    void GrService::OnSessionLogon(int id) {

    }

    void GrService::OnSessionLogoff(int id) {

    }

    void GrService::OnSessionLock(int id) {

    }

    void GrService::OnSessionUnlock(int id) {

    }

}
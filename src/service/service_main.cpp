//
// Created by RGAA on 21/10/2024.
//

#include "service_main.h"
#include "service_context.h"
#include "service.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_ext.h"
#include <string>
#include <Windows.h>
#include <Shlwapi.h>

using namespace tc;

#pragma comment(lib, "Shlwapi.lib")

std::shared_ptr<ServiceContext> g_context_ = nullptr;
std::shared_ptr<GrService> g_service_ = nullptr;

const std::string kGrServiceName = "GammaRayService";

std::wstring GetModulePathW(HMODULE hModule)
{
    const int maxPath = 4096;
    wchar_t szFullPath[maxPath] = { 0 };
    ::GetModuleFileNameW(hModule, szFullPath, maxPath);
    ::PathRemoveFileSpecW(szFullPath);
    return {szFullPath};
}

std::string GetModulePath(HMODULE hModule)
{
    return StringExt::ToUTF8(GetModulePathW(hModule));
}

DWORD WINAPI ServiceCtrlHandler(DWORD dwCtrl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {
    if (!g_service_) {
        return NO_ERROR;
    }
    switch (dwCtrl)
    {
        case SERVICE_CONTROL_STOP: {
            g_service_->OnCtrlStop();
            break;
        }

        case SERVICE_CONTROL_CONTINUE: {
            g_service_->OnCtrlContinue();
            break;
        }

        case SERVICE_CONTROL_PAUSE: {
            g_service_->OnCtrlPause();
            break;
        }

        case SERVICE_CONTROL_INTERROGATE: {
            g_service_->OnCtrlInterrogate();
            break;
        }

        // https://learn.microsoft.com/en-us/windows/win32/termserv/wm-wtssession-change
        case SERVICE_CONTROL_SESSIONCHANGE: {
            LOGI("WinService SERVICE_CONTROL_SESSIONCHANGE.");
            auto session_id = 0;
            if (lpEventData) {
                auto data = (WTSSESSION_NOTIFICATION*)lpEventData;
                session_id = data->dwSessionId;
            }

            if (dwEventType == WTS_CONSOLE_CONNECT) {
                g_service_->OnConsoleConnect(session_id);
            }
            if (dwEventType == WTS_CONSOLE_DISCONNECT) {
                g_service_->OnConsoleDisConnect(session_id);
            }
            if (dwEventType == WTS_REMOTE_CONNECT) {

            }
            if (dwEventType == WTS_REMOTE_DISCONNECT) {

            }
            if (dwEventType == WTS_SESSION_LOGON) {
                g_service_->OnSessionLogon(session_id);
            }
            if (dwEventType == WTS_SESSION_LOGOFF) {
                g_service_->OnSessionLogoff(session_id);
            }
            if (dwEventType == WTS_SESSION_LOCK) {
                g_service_->OnSessionLock(session_id);
            }
            if (dwEventType == WTS_SESSION_UNLOCK) {
                g_service_->OnSessionUnlock(session_id);
            }
            if (dwEventType == WTS_SESSION_REMOTE_CONTROL) {

            }
            if (dwEventType == WTS_SESSION_CREATE) {

            }
            if (dwEventType == WTS_SESSION_TERMINATE) {

            }
            break;
        }
        default:
            break;
    }
    return NO_ERROR;
}

void WINAPI ServiceMain(DWORD argc, LPWSTR* argv) {
    auto status_handle = RegisterServiceCtrlHandlerExA((char*)kGrServiceName.c_str(), ServiceCtrlHandler, nullptr);
    if (status_handle == nullptr) {
        LOGE("RegisterServiceCtrlHandler failed!");
        return;
    }
    g_service_->Run(argc, argv, status_handle);
}

int main(int argc, char** argv) {
    auto path = std::string(GetModulePath(nullptr)) + "/GammaRayService.log";
    Logger::InitLog(path, true);
    LOGI("----------Service Start----------");
    g_context_ = std::make_shared<ServiceContext>();
    g_service_ = std::make_shared<GrService>(g_context_);
    SERVICE_TABLE_ENTRY ServiceTable[] = {
            {(wchar_t*)kGrServiceName.c_str(), (LPSERVICE_MAIN_FUNCTION)ServiceMain},
            {nullptr, nullptr}
    };
    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
        LOGE("StartServiceCtrlDispatcher failed: {}", GetLastError());
        return GetLastError();
    }
    return 0;
}
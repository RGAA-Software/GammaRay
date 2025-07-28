//
// Created by RGAA on 22/10/2024.
//

#include "service_manager.h"
#include <format>
#include <iostream>
#include <QString>
#include <windows.h>

#include "tc_common_new/log.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/defer.h"

#pragma comment(lib, "Advapi32.lib")

namespace tc
{
    std::shared_ptr<ServiceManager> ServiceManager::Make() {
        return std::make_shared<ServiceManager>();
    }

    ServiceManager::ServiceManager() {

    }

    void ServiceManager::Init(const std::string &srv_name, const std::string &path, const std::string &display_name,
                              const std::string &description) {
        this->srv_name_ = srv_name;
        this->srv_exe_path_ = path;
        this->srv_display_name_ = display_name;
        this->srv_description_ = description;
    }

    void ServiceManager::Install() {
        SC_HANDLE schSCManager = nullptr;
        SC_HANDLE schService = nullptr;

        auto defer = Defer::Make([&]() {
            if (schService) {
                LOGI("Close service.");
                CloseServiceHandle(schService);
            }
            if (schSCManager) {
                LOGI("Close SCManager.");
                CloseServiceHandle(schSCManager);
            }
        });

        schSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (nullptr == schSCManager) {
            LOGE("OpenSCManager failed {}", GetLastError());
            return;
        }

        bool need_install = true;
        schService = OpenServiceW(schSCManager, QString::fromStdString(this->srv_name_).toStdWString().c_str(),
                                  SERVICE_ALL_ACCESS);
        if (schService != nullptr) {
            need_install = false;
            LOGI("OpenService success, don't need to install service");

            SERVICE_STATUS_PROCESS ssStatus;
            DWORD dwBytesNeeded;
            if (!QueryServiceStatusEx(
                    schService,                     // handle to service
                    SC_STATUS_PROCESS_INFO,         // information level
                    (LPBYTE) &ssStatus,             // address of structure
                    sizeof(SERVICE_STATUS_PROCESS), // size of structure
                    &dwBytesNeeded))              // size needed if buffer is too small
            {
                LOGE("QueryServiceStatusEx failed {}", GetLastError());
                return;
            }

            if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING) {
                LOGI("Service is already running.");
                return;
            }
        }

        if (need_install) {
            schService = CreateServiceW(
                    schSCManager,              // SCM database
                    QString::fromStdString(this->srv_name_).toStdWString().c_str(), // name of service
                    QString::fromStdString(this->srv_name_).toStdWString().c_str(), // service name to display
                    SERVICE_ALL_ACCESS,        // desired access
                    SERVICE_WIN32_OWN_PROCESS, // service type
                    SERVICE_AUTO_START,      // start type
                    SERVICE_ERROR_NORMAL,      // error control type
                    QString::fromStdString(this->srv_exe_path_).toStdWString().c_str(), // path to service's binary
                    nullptr,                      // no load ordering group
                    nullptr,                      // no tag identifier
                    nullptr,                      // no dependencies
                    nullptr,                      // LocalSystem account
                    nullptr);                     // no password


            if (schService == nullptr) {
                LOGE("CreateService failed {}", GetLastError());
                return;
            } else {
                LOGI("Service installed successfully");
            }
        }

        if (schService != nullptr) {
            if (!StartService(schService, 0, nullptr)) {
                LOGI("StartService failed {}", GetLastError());
                return;
            } else {
                LOGI("Service start pending.");
            }

            // restart config
            SERVICE_FAILURE_ACTIONS failureActions;
            failureActions.dwResetPeriod = 600;
            failureActions.lpRebootMsg = NULL;
            failureActions.lpCommand = NULL;
            failureActions.cActions = 1; // action count
            SC_ACTION restartAction;
            restartAction.Type = SC_ACTION_RESTART;
            restartAction.Delay = 3000; // 3000ms to restart
            failureActions.lpsaActions = &restartAction;

            ChangeServiceConfig2W(schService, SERVICE_CONFIG_FAILURE_ACTIONS, &failureActions);
        }
    }

    static BOOL __stdcall StopDependentServices(SC_HANDLE schSCManager, SC_HANDLE schService) {
        DWORD i;
        DWORD dwBytesNeeded;
        DWORD dwCount;

        LPENUM_SERVICE_STATUS lpDependencies = nullptr;
        ENUM_SERVICE_STATUS ess;
        SC_HANDLE hDepService;
        SERVICE_STATUS_PROCESS ssp;

        DWORD dwStartTime = GetTickCount();
        DWORD dwTimeout = 30000; // 30-second time-out

        // Pass a zero-length buffer to get the required buffer size.
        if (EnumDependentServices(schService, SERVICE_ACTIVE,
                                  lpDependencies, 0, &dwBytesNeeded, &dwCount)) {
            // If the Enum call succeeds, then there are no dependent
            // services, so do nothing.
            return TRUE;
        } else {
            if (GetLastError() != ERROR_MORE_DATA)
                return FALSE; // Unexpected error

            // Allocate a buffer for the dependencies.
            lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc(
                    GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

            if (!lpDependencies)
                return FALSE;

            __try{
                    // Enumerate the dependencies.
                    if ( !EnumDependentServices( schService, SERVICE_ACTIVE,
                    lpDependencies, dwBytesNeeded, &dwBytesNeeded,
                    &dwCount ))
                    return FALSE;

                    for ( i = 0; i < dwCount; i++ )
                    {
                        ess = *(lpDependencies + i);
                        // Open the service.
                        hDepService = OpenService(schSCManager,
                                                  ess.lpServiceName,
                                                  SERVICE_STOP | SERVICE_QUERY_STATUS);

                        if (!hDepService)
                            return FALSE;

                        __try{
                                // Send a stop code.
                                if ( !ControlService( hDepService,
                                SERVICE_CONTROL_STOP,
                                (LPSERVICE_STATUS) &ssp ))
                                return FALSE;

                                // Wait for the service to stop.
                                while ( ssp.dwCurrentState != SERVICE_STOPPED )
                                {
                                    Sleep(ssp.dwWaitHint);
                                    if (!QueryServiceStatusEx(
                                            hDepService,
                                            SC_STATUS_PROCESS_INFO,
                                            (LPBYTE) &ssp,
                                            sizeof(SERVICE_STATUS_PROCESS),
                                            &dwBytesNeeded))
                                        return FALSE;

                                    if (ssp.dwCurrentState == SERVICE_STOPPED)
                                        break;

                                    if (GetTickCount() - dwStartTime > dwTimeout)
                                        return FALSE;
                                }
                        }
                        __finally
                        {
                            // Always release the service handle.
                            CloseServiceHandle(hDepService);
                        }
                    }
            }
            __finally
            {
                // Always free the enumeration buffer.
                HeapFree(GetProcessHeap(), 0, lpDependencies);
            }
        }
        return TRUE;
    }

    static VOID __stdcall DoDeleteSvc(SC_HANDLE schSCManager, SC_HANDLE schService) {
        SERVICE_STATUS ssStatus;
        // Delete the service.
        if (!DeleteService(schService)) {
            LOGE("DeleteService failed {}", GetLastError());
        } else {
            LOGI("Service deleted successfully");
        }
    }

    void ServiceManager::Remove() {
        SC_HANDLE schSCManager = nullptr;
        SC_HANDLE schService = nullptr;
        SERVICE_STATUS_PROCESS ssp;
        DWORD dwStartTime = GetTickCount();
        DWORD dwBytesNeeded;
        DWORD dwTimeout = 30000; // 30-second time-out
        DWORD dwWaitTime;

        auto defer = Defer::Make([&]() {
            if (schSCManager && schService) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                DoDeleteSvc(schSCManager, schService);
            }
            if (schService) {
                LOGI("Close service.");
                CloseServiceHandle(schService);
            }
            if (schSCManager) {
                LOGI("Close SCManager.");
                CloseServiceHandle(schSCManager);
            }
        });

        // Get a handle to the SCM database.
        schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
        if (nullptr == schSCManager) {
            LOGE("OpenSCManager failed {}", GetLastError());
            return;
        }

        // Get a handle to the service.
        schService = OpenService(
                schSCManager,         // SCM database
                QString::fromStdString(this->srv_name_).toStdWString().c_str(),            // name of service
                SERVICE_STOP |
                SERVICE_QUERY_STATUS |
                SERVICE_ENUMERATE_DEPENDENTS |
                DELETE);

        if (schService == nullptr) {
            LOGE("OpenService failed {}", GetLastError());
            CloseServiceHandle(schSCManager);
            return;
        }

        // Make sure the service is not already stopped.
        if (!QueryServiceStatusEx(
                schService,
                SC_STATUS_PROCESS_INFO,
                (LPBYTE) &ssp,
                sizeof(SERVICE_STATUS_PROCESS),
                &dwBytesNeeded)) {
            LOGE("QueryServiceStatusEx failed {}", GetLastError());
            return;
        }

        if (ssp.dwCurrentState == SERVICE_STOPPED) {
            LOGI("Service is already stopped, will delete it");
            DoDeleteSvc(schSCManager, schService);
            return;
        }

        // If a stop is pending, wait for it.
        while (ssp.dwCurrentState == SERVICE_STOP_PENDING) {
            LOGI("Service stop pending...");

            // Do not wait longer than the wait hint. A good interval is
            // one-tenth of the wait hint but not less than 1 second
            // and not more than 10 seconds.

            dwWaitTime = ssp.dwWaitHint / 10;

            if (dwWaitTime < 1000)
                dwWaitTime = 1000;
            else if (dwWaitTime > 10000)
                dwWaitTime = 10000;

            Sleep(dwWaitTime);

            if (!QueryServiceStatusEx(
                    schService,
                    SC_STATUS_PROCESS_INFO,
                    (LPBYTE) &ssp,
                    sizeof(SERVICE_STATUS_PROCESS),
                    &dwBytesNeeded)) {
                LOGI("QueryServiceStatusEx failed {}", GetLastError());
                break;
            }

            if (ssp.dwCurrentState == SERVICE_STOPPED) {
                LOGI("Service stopped successfully. Will delete it");
                break;
            }

            if (GetTickCount() - dwStartTime > dwTimeout) {
                LOGW("Service stop timed out.");
                break;
            }
        }

        // If the service is running, dependencies must be stopped first.
        StopDependentServices(schSCManager, schService);

        // Send a stop code to the service.

        if (!ControlService(schService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS) &ssp)) {
            LOGE("ControlService failed: {}", GetLastError());
            return;
        }

        // Wait for the service to stop.
        while (ssp.dwCurrentState != SERVICE_STOPPED) {
            Sleep(ssp.dwWaitHint);
            if (!QueryServiceStatusEx(
                    schService,
                    SC_STATUS_PROCESS_INFO,
                    (LPBYTE) &ssp,
                    sizeof(SERVICE_STATUS_PROCESS),
                    &dwBytesNeeded)) {
                printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
                return;
            }

            if (ssp.dwCurrentState == SERVICE_STOPPED)
                break;

            if (GetTickCount() - dwStartTime > dwTimeout) {
                printf("Wait timed out\n");
                return;
            }
        }
        LOGI("Service stopped successfully\n");
    }


    ServiceStatus ServiceManager::QueryStatus() {
        auto lines = ProcessUtil::StartProcessAndOutput("sc", {"query", this->srv_name_});
        ServiceStatus status = ServiceStatus::kUnknownStatus;
        for (auto &l: lines) {
            auto line = QString::fromStdString(l);

            std::cout << "line: " << l << std::endl;
            if (!line.contains("STATE")) {
                continue;
            }
            if (line.contains("STOPPED")) {
                status = ServiceStatus::kStopped;
            } else if (line.contains("PENDING")) {
                status = ServiceStatus::kPending;
            } else if (line.contains("RUNNING")) {
                status = ServiceStatus::kRunning;
            }
        }
        return status;
    }

    std::string ServiceManager::StatusAsString(ServiceStatus status) {
        if (status == ServiceStatus::kPending) {
            return "pending";
        } else if (status == ServiceStatus::kStopped) {
            return "stopped";
        } else if (status == ServiceStatus::kRunning) {
            return "running";
        } else {
            return "unknown";
        }
    }
}
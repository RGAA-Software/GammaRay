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

#pragma comment(lib, "Advapi32.lib")

namespace tc
{
    std::shared_ptr<ServiceManager> ServiceManager::Make() {
        return std::make_shared<ServiceManager>();
    }

    ServiceManager::ServiceManager() {

    }

    void ServiceManager::Init(const std::string& srv_name, const std::string& path, const std::string& display_name, const std::string& description) {
        this->srv_name_ = srv_name;
        this->srv_exe_path_ = path;
        this->srv_display_name_ = display_name;
        this->srv_description_ = description;
    }

    void ServiceManager::Install() {
        SC_HANDLE schSCManager;
        SC_HANDLE schService;

        schSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (NULL == schSCManager) {
            LOGE("OpenSCManager failed {}", GetLastError());
            return;
        }

        schService = CreateServiceW(
                schSCManager,              // SCM database
                QString::fromStdString(this->srv_name_).toStdWString().c_str(),                   // name of service
                QString::fromStdString(this->srv_name_).toStdWString().c_str(),                   // service name to display
                SERVICE_ALL_ACCESS,        // desired access
                SERVICE_WIN32_OWN_PROCESS, // service type
                SERVICE_AUTO_START,      // start type
                SERVICE_ERROR_NORMAL,      // error control type
                QString::fromStdString(this->srv_exe_path_).toStdWString().c_str(),                    // path to service's binary
                NULL,                      // no load ordering group
                NULL,                      // no tag identifier
                NULL,                      // no dependencies
                NULL,                      // LocalSystem account
                NULL);                     // no password


        if (schService == NULL) {
            LOGE("CreateService failed {}", GetLastError());
            CloseServiceHandle(schSCManager);
            return;
        }
        else {
            LOGI("Service installed successfully");
        }

        if (!StartService(schService, 0, NULL)) {
            LOGI("StartService failed {}", GetLastError());
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return;
        }
        else {
            LOGI("Service start pending.");
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }

    void ServiceManager::Start() {
        auto cmd = std::format(R"(sc start {}")", this->srv_name_);
        std::cout << "cmd: " << cmd << std::endl;
        auto lines = ProcessUtil::StartProcessAndOutput(cmd, {});
    }

    void ServiceManager::Stop() {
        auto cmd = std::format(R"(sc stop {}")", this->srv_name_);
        std::cout << "cmd: " << cmd << std::endl;
        auto lines = ProcessUtil::StartProcessAndOutput(cmd, {});
    }

    void ServiceManager::Remove() {
        auto cmd = std::format(R"(sc delete {}")", this->srv_name_);
        std::cout << "cmd: " << cmd << std::endl;
        auto lines = ProcessUtil::StartProcessAndOutput(cmd, {});
    }

    void ServiceManager::RemoveImmediately() {
        std::cout << "remove immediately..." << std::endl;
        this->Stop();
        this->Remove();
        this->QueryStatus();
    }

    ServiceStatus ServiceManager::QueryStatus() {
        auto cmd = std::format(R"(sc query {}")", this->srv_name_);
        std::cout << "cmd: " << cmd << std::endl;
        auto lines = ProcessUtil::StartProcessAndOutput(cmd, {});
        ServiceStatus status = ServiceStatus::kUnknownStatus;
        for (auto& l : lines) {
            auto line = QString::fromStdString(l);

            std::cout << "line: " << l << std::endl;
            if (!line.contains("STATE")) {
                continue;
            }
            if (line.contains("STOPPED")) {
                status = ServiceStatus::kStopped;
            }
            else if (line.contains("PENDING")) {
                status = ServiceStatus::kPending;
            }
            else if (line.contains("RUNNING")) {
                status = ServiceStatus::kRunning;
            }
        }
        return status;
    }

    std::string ServiceManager::StatusAsString(ServiceStatus status) {
        if (status == ServiceStatus::kPending) {
            return "pending";
        }
        else if (status == ServiceStatus::kStopped) {
            return "stopped";
        }
        else if (status == ServiceStatus::kRunning) {
            return "running";
        }
        else {
            return "unknown";
        }
    }
}
//
// Created by RGAA on 22/10/2024.
//

#include "service_manager.h"
#include <format>
#include <iostream>
#include <QString>

#include "tc_common_new/log.h"
#include "tc_common_new/process_util.h"

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
        auto cmd = std::format(R"(sc create {} binPath= "{}" start=auto DisplayName="{}")",
                               this->srv_name_, this->srv_exe_path_, this->srv_display_name_);
        std::cout << "==> create cmd: " << cmd << std::endl;
        auto lines = ProcessUtil::StartProcessAndOutput(cmd, {});
        std::cout << "install result: \n";
        for (auto& line : lines) {
            std::cout << "line: " << line << std::endl;
        }

        auto config = std::format(R"(sc config {} start= auto)", this->srv_name_);
        std::cout << "==> config cmd: " << config << std::endl;
        lines.clear();
        lines = ProcessUtil::StartProcessAndOutput(config, {});
        std::cout << "config result: \n";
        for (auto& line : lines) {
            std::cout << "line: " << line << std::endl;
        }

        config = std::format(R"(sc failure {} reset= 0 actions= restart/1000)", this->srv_name_);
        std::cout << "==> config failure action: " << config << std::endl;
        lines.clear();
        lines = ProcessUtil::StartProcessAndOutput(config, {});
        std::cout << "config result: \n";
        for (auto& line : lines) {
            std::cout << "line: " << line << std::endl;
        }

        auto desc = std::format(R"(sc description {} "{}")", this->srv_name_, this->srv_description_);
        std::cout << "==> desc cmd: " << desc << std::endl;
        lines.clear();
        lines = ProcessUtil::StartProcessAndOutput(desc, {});
        std::cout << "desc result: " << std::endl;
        for (auto& line : lines) {
            std::cout << "line: " << line << std::endl;
        }

        auto status = this->QueryStatus();
        if (status != ServiceStatus::kRunning) {
            std::cout << "service is not in running state, will start it" << std::endl;
            this->Start();
            std::cout << "re-check service state:" << std::endl;
            this->QueryStatus();
        }
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
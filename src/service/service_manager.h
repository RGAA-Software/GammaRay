//
// Created by RGAA on 22/10/2024.
//

#ifndef GAMMARAY_SERVICE_MANAGER_H
#define GAMMARAY_SERVICE_MANAGER_H

#include <memory>
#include <string>

namespace tc
{
    enum class ServiceStatus {
        kUnknownStatus,
        kPending,
        kRunning,
        kStopped,
    };

    class ServiceManager {
    public:
        static std::shared_ptr<ServiceManager> Make();
        ServiceManager();

        void Init(const std::string& srv_name, const std::string& path, const std::string& display_name, const std::string& description);
        void Install();
        void Remove();
        ServiceStatus QueryStatus();

        static std::string StatusAsString(ServiceStatus status);

    private:
        std::string srv_name_;
        std::string srv_exe_path_;
        std::string srv_display_name_;
        std::string srv_description_;
    };

}

#endif //GAMMARAY_SERVICE_MANAGER_H

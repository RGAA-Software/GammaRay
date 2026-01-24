//
// Created by RGAA on 23/01/2026.
//

#ifndef GAMMARAYPREMIUM_GR_EVENT_MANAGER_H
#define GAMMARAYPREMIUM_GR_EVENT_MANAGER_H

#include <memory>
#include <string>

namespace tc
{

    class GrContext;
    class GrSettings;
    class GrUserManager;

    class GrEventManager {
    public:
        explicit GrEventManager(const std::shared_ptr<GrContext>& context);

        // add cpu event
        bool AddCpuEvent(int cpu_usage);

        // add memory event
        bool AddMemoryEvent(int memory_usage);

        // add disk event
        bool AddDiskEvent(int disk_usage, const std::string& disk_path);

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<GrUserManager> user_mgr_ = nullptr;
    };

}

#endif //GAMMARAYPREMIUM_GR_EVENT_MANAGER_H
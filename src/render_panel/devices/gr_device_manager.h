//
// Created by RGAA on 27/11/2025.
//

#ifndef GAMMARAYPREMIUM_GR_DEVICE_MANAGER_H
#define GAMMARAYPREMIUM_GR_DEVICE_MANAGER_H

#include <memory>
#include <string>

namespace tc
{

    class GrContext;
    class GrSettings;

    class GrDeviceManager {
    public:
        explicit GrDeviceManager(const std::shared_ptr<GrContext>& ctx);

        bool UpdateDesktopLink(const std::string& desktop_link, const std::string& desktop_link_raw);

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;

    };

}

#endif //GAMMARAYPREMIUM_GR_DEVICE_MANAGER_H

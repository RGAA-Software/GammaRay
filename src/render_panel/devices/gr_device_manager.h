//
// Created by RGAA on 27/11/2025.
//

#ifndef GAMMARAYPREMIUM_GR_DEVICE_MANAGER_H
#define GAMMARAYPREMIUM_GR_DEVICE_MANAGER_H

#include <memory>
#include <string>

#include "expt/expected.h"
#include "tc_spvr_client/spvr_errors.h"

namespace spvr
{
    class SpvrDevice;
}

namespace tc
{

    class GrContext;
    class GrSettings;

    class GrDeviceManager {
    public:
        explicit GrDeviceManager(const std::shared_ptr<GrContext>& ctx);
        // request new device
        // def_device_name: D-{last segment of ip}
        // info: empty
        Result<std::shared_ptr<spvr::SpvrDevice>, spvr::SpvrApiError> RequestNewDevice(const std::string& def_device_name, const std::string& info);

        // query device
        Result<std::shared_ptr<spvr::SpvrDevice>, spvr::SpvrApiError> QueryDevice(const std::string& device_id);

        // update desktop link to device
        bool UpdateDesktopLink(const std::string& desktop_link, const std::string& desktop_link_raw);

        // update device name
        Result<std::shared_ptr<spvr::SpvrDevice>, spvr::SpvrApiError> UpdateDeviceName(const std::string& device_name);

        // append used time
        Result<std::shared_ptr<spvr::SpvrDevice>, spvr::SpvrApiError> UpdateUsedTime(int period);

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;

    };

}

#endif //GAMMARAYPREMIUM_GR_DEVICE_MANAGER_H

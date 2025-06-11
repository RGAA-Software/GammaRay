//
// Created by RGAA on 3/03/2025.
//

#ifndef GAMMARAY_GR_PLUGIN_SETTINGS_INFO_H
#define GAMMARAY_GR_PLUGIN_SETTINGS_INFO_H

#include <string>

namespace tc
{

    class GrPluginSettingsInfo {
    public:
        // this device, device id
        std::string device_id_;
        std::string device_random_pwd_;
        std::string device_safety_pwd_;
        // relay host
        std::string relay_host_;
        // relay port
        std::string relay_port_;
    };

}

#endif //GAMMARAY_GR_PLUGIN_SETTINGS_INFO_H

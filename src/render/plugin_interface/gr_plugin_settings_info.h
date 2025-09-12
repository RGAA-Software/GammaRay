//
// Created by RGAA on 3/03/2025.
//

#ifndef GAMMARAY_GR_PLUGIN_SETTINGS_INFO_H
#define GAMMARAY_GR_PLUGIN_SETTINGS_INFO_H

#include <string>

namespace tc
{

    // from render panel -> render
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
        // can be operated by mouse and keyboard
        bool can_be_operated_ = true;
        // relay enabled
        bool relay_enabled_ = true;
        // language // default is English
        int language_ = 1;
        // file transfer enabled or not
        bool file_transfer_enabled_ = true;
        // audio enabled or not
        bool audio_enabled_ = true;
        // appkey
        std::string appkey_;
    };

}

#endif //GAMMARAY_GR_PLUGIN_SETTINGS_INFO_H

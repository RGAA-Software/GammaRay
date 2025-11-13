//
// Created by RGAA on 25/06/2025.
//

#ifndef GAMMARAY_CT_PLUGIN_SETTINGS_H
#define GAMMARAY_CT_PLUGIN_SETTINGS_H

#include <string>

namespace tc
{

    class SkinSettings {
    public:
        bool clipboard_enabled_ = false;
        std::string device_id_;
        std::string stream_id_;
        int language_ = 1;
        std::string stream_name_;
        std::string display_name_;
        std::string display_remote_name_;
        uint64_t max_transmit_speed_ = 0;
        uint64_t max_receive_speed_ = 0;
    };

}

#endif //GAMMARAY_CT_PLUGIN_SETTINGS_H

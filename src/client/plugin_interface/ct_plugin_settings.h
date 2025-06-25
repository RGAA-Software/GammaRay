//
// Created by RGAA on 25/06/2025.
//

#ifndef GAMMARAY_CT_PLUGIN_SETTINGS_H
#define GAMMARAY_CT_PLUGIN_SETTINGS_H

#include <string>

namespace tc
{

    class ClientPluginSettings {
    public:
        bool clipboard_enabled_ = false;
        std::string device_id_;
        std::string stream_id_;
    };

}

#endif //GAMMARAY_CT_PLUGIN_SETTINGS_H

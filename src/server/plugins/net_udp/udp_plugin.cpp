//
// Created RGAA on 15/11/2024.
//

#include "udp_plugin.h"

namespace tc
{

    std::string UdpPlugin::GetPluginName() {
        return "Udp Plugin";
    }

    std::string UdpPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t UdpPlugin::GetVersionCode() {
        return 110;
    }

}

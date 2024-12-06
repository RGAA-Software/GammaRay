//
// Created RGAA on 15/11/2024.
//

#include "udp_plugin.h"
#include "render/plugins/plugin_ids.h"

namespace tc
{

    std::string UdpPlugin::GetPluginId() {
        return kNetUdpPluginId;
    }

    std::string UdpPlugin::GetPluginName() {
        return "UDP Plugin";
    }

    std::string UdpPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t UdpPlugin::GetVersionCode() {
        return 110;
    }

}

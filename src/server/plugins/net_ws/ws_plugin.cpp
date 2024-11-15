//
// Created RGAA on 15/11/2024.
//

#include "ws_plugin.h"
#include "plugin_interface/gr_plugin_events.h"

namespace tc
{

    std::string WsPlugin::GetPluginName() {
        return "Ws Plugin";
    }

    std::string WsPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t WsPlugin::GetVersionCode() {
        return 110;
    }

    void WsPlugin::On1Second() {
        GrPluginInterface::On1Second();
        auto evt = std::make_shared<GrPluginKeyboardEvent>();
        evt->plugin_name_ = GetPluginName();
        CallbackEvent(evt);
    }

}

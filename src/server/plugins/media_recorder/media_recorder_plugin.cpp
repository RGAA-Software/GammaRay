//
// Created RGAA on 15/11/2024.
//

#include "media_recorder_plugin.h"
#include "plugin_interface/gr_plugin_events.h"

namespace tc
{

    std::string MediaRecorderPlugin::GetPluginName() {
        return "Media Recorder Plugin";
    }

    std::string MediaRecorderPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t MediaRecorderPlugin::GetVersionCode() {
        return 110;
    }

    void MediaRecorderPlugin::On1Second() {
        GrPluginInterface::On1Second();
        auto evt = std::make_shared<GrPluginKeyboardEvent>();
        evt->plugin_name_ = GetPluginName();
        CallbackEvent(evt);
    }

}

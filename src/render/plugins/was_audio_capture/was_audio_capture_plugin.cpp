//
// Created RGAA on 15/11/2024.
//

#include "was_audio_capture_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "render/plugins/plugin_ids.h"

namespace tc
{
    std::string WasAudioCapturePlugin::GetPluginId() {
        return kWasAudioCapturePluginId;
    }

    std::string WasAudioCapturePlugin::GetPluginName() {
        return "Was Audio Capture Plugin";
    }

    std::string WasAudioCapturePlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t WasAudioCapturePlugin::GetVersionCode() {
        return 110;
    }

    void WasAudioCapturePlugin::On1Second() {

    }

}

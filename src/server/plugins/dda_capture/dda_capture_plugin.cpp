//
// Created RGAA on 15/11/2024.
//

#include "dda_capture_plugin.h"

static void* GetInstance() {
    static tc::DDACapturePlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    DDACapturePlugin::DDACapturePlugin() : GrDataProviderPlugin() {

    }

    std::string DDACapturePlugin::GetPluginName() {
        return "DDA Capture Plugin";
    }

    std::string DDACapturePlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t DDACapturePlugin::GetVersionCode() {
        return 110;
    }

}

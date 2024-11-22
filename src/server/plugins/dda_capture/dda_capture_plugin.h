//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_DDA_CAPTURE_PLUGIN_H
#define GAMMARAY_DDA_CAPTURE_PLUGIN_H

#include "plugin_interface/gr_data_provider_plugin.h"

namespace tc
{

    class DDACapturePlugin : public GrDataProviderPlugin {
    public:
        DDACapturePlugin();
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;

    };

}

extern "C" __declspec(dllexport) void* GetInstance();


#endif //GAMMARAY_UDP_PLUGIN_H

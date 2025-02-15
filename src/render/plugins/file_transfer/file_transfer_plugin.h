//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_RTC_PLUGIN_H
#define GAMMARAY_RTC_PLUGIN_H

#include "plugin_interface/gr_net_plugin.h"

namespace tc
{

    class FileTransferPlugin : public GrNetPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        void OnProtoMessage(const std::string& msg) override;

    private:
        void PostTargetStreamMessage(const std::string& stream_id, const std::string& msg);

    };

}

extern "C" __declspec(dllexport) void* GetInstance();

#endif //GAMMARAY_UDP_PLUGIN_H

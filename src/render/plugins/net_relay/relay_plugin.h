//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_VR_MANAGER_PLUGIN_H
#define GAMMARAY_VR_MANAGER_PLUGIN_H

#include "plugin_interface/gr_net_plugin.h"

namespace tc
{

    class RelayClientSdk;

    class RelayPlugin : public GrNetPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        void On1Second() override;
        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        void PostProtoMessage(const std::string &msg) override;
        bool PostTargetStreamProtoMessage(const std::string& stream_id, const std::string& msg) override;
        int ConnectedClientSize() override;
        bool IsOnlyAudioClients() override;
        bool IsWorking() override;
        void SyncInfo(const tc::NetSyncInfo &info) override;

    private:
        std::shared_ptr<RelayClientSdk> relay_sdk_ = nullptr;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();


#endif //GAMMARAY_UDP_PLUGIN_H

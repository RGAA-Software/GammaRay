//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_VR_MANAGER_PLUGIN_H
#define GAMMARAY_VR_MANAGER_PLUGIN_H

#include "plugin_interface/gr_net_plugin.h"

namespace tc
{

    class RelayServerSdk;

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
        bool PostTargetFileTransferProtoMessage(const std::string &stream_id, const std::string &msg) override;
        int ConnectedClientSize() override;
        bool IsOnlyAudioClients() override;
        bool IsWorking() override;
        void SyncInfo(const tc::NetSyncInfo& info) override;
        void OnSyncSystemSettings(const tc::GrPluginSettingsInfo &settings) override;
        int64_t GetQueuingMediaMsgCount() override;
        int64_t GetQueuingFtMsgCount() override;

    private:
        void NotifyMediaClientConnected();
        void NotifyMediaClientDisConnected();

    private:
        std::shared_ptr<RelayServerSdk> relay_media_sdk_ = nullptr;
        std::shared_ptr<RelayServerSdk> relay_ft_sdk_ = nullptr;
        std::atomic_bool sdk_init_ = false;
        std::atomic_uint64_t recv_relay_ft_msg_index_ = 0;
        std::atomic_uint64_t recv_relay_media_msg_index_ = 0;

        // don't send media stream at begin
        // client will request to resume it
        bool paused_stream = true;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();


#endif //GAMMARAY_UDP_PLUGIN_H

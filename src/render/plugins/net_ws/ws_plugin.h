//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_WS_PLUGIN_H
#define GAMMARAY_WS_PLUGIN_H

#include "plugin_interface/gr_net_plugin.h"

namespace tc
{

    class WsPluginServer;

    class WsPlugin : public GrNetPlugin {
    public:
        WsPlugin();
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        std::string GetPluginDescription() override;
        bool OnCreate(const tc::GrPluginParam& param) override;
        bool OnDestroy() override;
        void On1Second() override;
        bool IsWorking() override;

        void PostProtoMessage(const std::string& msg, bool run_through) override;
        bool PostTargetStreamProtoMessage(const std::string& stream_id, const std::string& msg, bool run_through) override;
        bool PostTargetFileTransferProtoMessage(const std::string& stream_id, const std::string& msg, bool run_through) override;

        bool IsOnlyAudioClients() override;
        int GetConnectedPeerCount() override;

        int64_t GetQueuingMediaMsgCount() override;
        int64_t GetQueuingFtMsgCount() override;
        bool HasEnoughBufferForQueuingMediaMessages() override;
        bool HasEnoughBufferForQueuingFtMessages() override;

    private:
        bool HasConnectedClients();

    private:

        std::shared_ptr<WsPluginServer> ws_server_ = nullptr;

    };

}

extern "C" __declspec(dllexport) void* GetInstance();

#endif //GAMMARAY_UDP_PLUGIN_H

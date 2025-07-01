//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_UDP_PLUGIN_H
#define GAMMARAY_UDP_PLUGIN_H

#include "plugin_interface/gr_net_plugin.h"
#include <asio2/udp/udp_server.hpp>
#include "tc_common_new/concurrent_hashmap.h"

namespace tc
{
    class Data;

    struct UdpMessagePack {
        uint32_t magic_ = 0;
        uint32_t length_ = 0;
        // data below...
    };

    class UdpSession {
    public:
        int64_t socket_fd_{0};
        std::string device_id_;
        std::string stream_id_;
        std::shared_ptr<asio2::udp_session> sess_ = nullptr;
    };

    class UdpPlugin : public GrNetPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        std::string GetPluginDescription() override;

        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        void PostProtoMessage(const std::string &msg, bool run_through = false) override;
        bool PostTargetStreamProtoMessage(const std::string& stream_id, const std::string& msg, bool run_through = false) override;
        int GetConnectedClientsCount() override;
        bool IsOnlyAudioClients() override;
        bool IsWorking() override;
        void SyncInfo(const tc::NetSyncInfo &info) override;
        void NotifyMediaClientConnected();
        void NotifyMediaClientDisConnected();

        bool HasEnoughBufferForQueuingMediaMessages() override;
        bool HasEnoughBufferForQueuingFtMessages() override;

    private:
        void StartInternal();

    private:
        std::shared_ptr<asio2::udp_server> server_ = nullptr;
        int udp_listen_port_{};
        tc::ConcurrentHashMap<int64_t, std::shared_ptr<UdpSession>> sessions_;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

void* GetInstance() {
    static tc::UdpPlugin plugin;
    return (void*)&plugin;
}


#endif //GAMMARAY_UDP_PLUGIN_H

//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_RTC_PLUGIN_H
#define GAMMARAY_RTC_PLUGIN_H

#include "plugin_interface/gr_net_plugin.h"
#include "rtc_messages.h"
#include "tc_common_new/concurrent_hashmap.h"

namespace tc
{

    class RtcServer;

    class RtcPlugin : public GrNetPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        bool OnCreate(const tc::GrPluginParam &param) override;
        void OnMessageRaw(const std::any &msg) override;
        void PostProtoMessage(const std::string &msg, bool run_through) override;
        bool PostTargetStreamProtoMessage(const std::string &stream_id, const std::string &msg, bool run_through) override;
        bool PostTargetFileTransferProtoMessage(const std::string &stream_id, const std::string &msg, bool run_through) override;
        int ConnectedClientSize() override;

    private:
        void OnRemoteSdp(const MsgRtcRemoteSdp& m);
        void OnRemoteIce(const MsgRtcRemoteIce& m);

    private:
        ConcurrentHashMap<std::string, std::shared_ptr<RtcServer>> rtc_servers_;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();


#endif //GAMMARAY_UDP_PLUGIN_H

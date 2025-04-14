//
// Created RGAA on 15/11/2024.
//

#include "rtc_plugin.h"
#include "render/plugins/plugin_ids.h"
#include "video_source_mock.h"
#include "tc_common_new/log.h"
#include "rtc_messages.h"

namespace tc
{

    std::string RtcPlugin::GetPluginId() {
        return kNetRtcPluginId;
    }

    std::string RtcPlugin::GetPluginName() {
        return "RTC Plugin";
    }

    std::string RtcPlugin::GetVersionName() {
        return "1.0.2";
    }

    uint32_t RtcPlugin::GetVersionCode() {
        return 102;
    }

    bool RtcPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrNetPlugin::OnCreate(param);
        return true;
    }

    void RtcPlugin::OnMessageRaw(const std::any& msg) {
        if (HoldsType<MsgRtcRemoteSdp>(msg)) {
            auto m = std::any_cast<MsgRtcRemoteSdp>(msg);
            LOGI("==> Offer sdp {} => {}", m.stream_id_, m.sdp_);
        }
        else if (HoldsType<MsgRtcRemoteIce>(msg)) {
            auto m = std::any_cast<MsgRtcRemoteIce>(msg);
            LOGI("==> Ice {} => {}", m.stream_id_, m.ice_);
        }
    }

    void RtcPlugin::PostProtoMessage(const std::string& msg) {

    }

}

void* GetInstance() {
    static tc::RtcPlugin plugin;
    return (void*)&plugin;
}
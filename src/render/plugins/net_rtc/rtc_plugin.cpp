//
// Created RGAA on 15/11/2024.
//

#include "rtc_plugin.h"
#include "render/plugins/plugin_ids.h"
#include "video_source_mock.h"
#include "tc_common_new/log.h"
#include "rtc_messages.h"
#include "rtc_server.h"

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
            this->OnRemoteSdp(m);
        }
        else if (HoldsType<MsgRtcRemoteIce>(msg)) {
            auto m = std::any_cast<MsgRtcRemoteIce>(msg);
            this->OnRemoteIce(m);
        }
    }

    void RtcPlugin::PostProtoMessage(const std::string& msg, bool run_through) {
        rtc_servers_.ApplyAll([=, this](const std::string& k, const std::shared_ptr<RtcServer>& srv) {
            PostWorkTask([=, this]() {
                srv->PostProtoMessage(msg, run_through);
            });
        });
    }

    bool RtcPlugin::PostTargetStreamProtoMessage(const std::string &stream_id, const std::string &msg, bool run_through) {
        rtc_servers_.ApplyAll([=, this](const std::string& k, const std::shared_ptr<RtcServer>& srv) {
            PostWorkTask([=, this]() {
                srv->PostTargetStreamProtoMessage(stream_id, msg, run_through);
            });
        });
        return true;
    }

    bool RtcPlugin::PostTargetFileTransferProtoMessage(const std::string &stream_id, const std::string &msg, bool run_through) {
        rtc_servers_.ApplyAll([=, this](const std::string& k, const std::shared_ptr<RtcServer>& srv) {
            PostWorkTask([=, this]() {
                srv->PostTargetFileTransferProtoMessage(stream_id, msg, run_through);
            });
        });
        return true;
    }

    void RtcPlugin::OnRemoteSdp(const MsgRtcRemoteSdp& m) {
        PostWorkTask([=, this]() {
            auto conn_id = m.device_id_ + ":" + m.stream_id_;
            LOGI("==>OnRemote Offer sdp {} => {}", conn_id, m.sdp_.size());
            auto opt_rtc_server = rtc_servers_.TryGet(conn_id);
            if (opt_rtc_server.has_value()) {
                LOGI("Remove old one.");
                opt_rtc_server.value()->Exit();
                rtc_servers_.Remove(conn_id);
            }

            auto rtc_server = RtcServer::Make(this);
            rtc_server->Start(m.stream_id_, m.sdp_);
            rtc_servers_.Insert(conn_id, rtc_server);
        });
    }

    void RtcPlugin::OnRemoteIce(const MsgRtcRemoteIce& m) {
        PostWorkTask([=, this]() {
            auto conn_id = m.device_id_ + ":" + m.stream_id_;
            if (auto opt_rtc_server = rtc_servers_.TryGet(conn_id); opt_rtc_server.has_value()) {
                auto rtc_server = opt_rtc_server.value();
                rtc_server->OnRemoteIce(m.ice_, m.mid_, m.sdp_mline_index_);
            }
        });
    }

    int RtcPlugin::ConnectedClientSize() {
        bool has_connected_channel_ = false;
        rtc_servers_.ApplyAll([&](const auto&, const std::shared_ptr<RtcServer>& srv) {
            if (srv->IsDataChannelConnected()) {
                has_connected_channel_ = true;
            }
        });
        return has_connected_channel_;
    }

}

void* GetInstance() {
    static tc::RtcPlugin plugin;
    return (void*)&plugin;
}
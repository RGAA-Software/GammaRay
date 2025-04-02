//
// Created RGAA on 15/11/2024.
//

#include "ws_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "ws_server.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "render/plugins/plugin_ids.h"
#include <QFile>

static void* GetInstance() {
    static tc::WsPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    WsPlugin::WsPlugin() : GrNetPlugin() {

    }

    std::string WsPlugin::GetPluginId() {
        return kNetWsPluginId;
    }

    std::string WsPlugin::GetPluginName() {
        return "WS Plugin";
    }

    std::string WsPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t WsPlugin::GetVersionCode() {
        return 110;
    }

    bool WsPlugin::OnCreate(const tc::GrPluginParam& param) {
        GrPluginInterface::OnCreate(param);
        auto listen_port = GetConfigIntParam("ws-listen-port");
        auto config_listen_port = GetConfigIntParam("listen-port");
        if (config_listen_port > 0) {
            listen_port = config_listen_port;
        }
        ws_server_ = std::make_shared<WsPluginServer>(this, (uint16_t)listen_port);
        ws_server_->Start();
        return true;
    }

    bool WsPlugin::OnDestroy() {
        if (ws_server_) {
            ws_server_->Exit();
        }
        return true;
    }

    void WsPlugin::On1Second() {

    }

    bool WsPlugin::IsWorking() {
        return ws_server_ && ws_server_->GetConnectionPeerCount() > 0;
    }

    void WsPlugin::PostProtoMessage(const std::string& msg) {
        if (IsWorking()) {
            ws_server_->PostNetMessage(msg);
        }
    }

    bool WsPlugin::PostTargetStreamProtoMessage(const std::string& stream_id, const std::string& msg) {
        if (!IsWorking()) {
            return false;
        }
        return ws_server_->PostTargetStreamMessage(stream_id, msg);
    }

    bool WsPlugin::PostTargetFileTransferProtoMessage(const std::string& stream_id, const std::string& msg) {
        if (!IsWorking()) {
            return false;
        }
        return ws_server_->PostTargetFileTransferMessage(stream_id, msg);
    }

    bool WsPlugin::IsOnlyAudioClients() {
        if (IsWorking()) {
            return ws_server_->IsOnlyAudioClients();
        } else {
            return false;
        }
    }

    int WsPlugin::ConnectedClientSize() {
        if (IsWorking()) {
            return ws_server_->GetConnectionPeerCount();
        } else {
            return 0;
        }
    }

    int64_t WsPlugin::GetQueuingMediaMsgCount() {
        if (IsWorking()) {
            return ws_server_->GetQueuingMediaMsgCount();
        } else {
            return 0;
        }
    }

    int64_t WsPlugin::GetQueuingFtMsgCount() {
        if (IsWorking()) {
            return ws_server_->GetQueuingFtMsgCount();
        } else {
            return 0;
        }
    }
}

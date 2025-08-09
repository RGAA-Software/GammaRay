//
// Created RGAA on 15/11/2024.
//

#include "ws_plugin.h"

#include <QFile>

#include "ws_server.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "render/plugins/plugin_ids.h"
#include "plugin_interface/gr_plugin_events.h"
#include "plugin_interface/gr_plugin_context.h"

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
        return "Net WebSocket";
    }

    std::string WsPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t WsPlugin::GetVersionCode() {
        return 110;
    }

    std::string WsPlugin::GetPluginDescription() {
        return "Network via WebSocket";
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
        return ws_server_ && ws_server_->IsWorking();
    }

    void WsPlugin::PostProtoMessage(std::shared_ptr<Data> msg, bool run_through) {
        if (IsWorking() && HasConnectedClients() && msg) {
            plugin_context_->PostWorkTask([=, this]() {
                ws_server_->PostNetMessage(msg);
            });
        }
    }

    bool WsPlugin::PostTargetStreamProtoMessage(const std::string& stream_id, std::shared_ptr<Data> msg, bool run_through) {
        if (IsWorking() && HasConnectedClients() && msg) {
            return ws_server_->PostTargetStreamMessage(stream_id, msg);
        }
        return false;
    }

    bool WsPlugin::PostTargetFileTransferProtoMessage(const std::string& stream_id, std::shared_ptr<Data> msg, bool run_through) {
        if (IsWorking() && HasConnectedClients() && msg) {
            return ws_server_->PostTargetFileTransferMessage(stream_id, msg);
        }
        return false;
    }

    bool WsPlugin::IsOnlyAudioClients() {
        if (IsWorking()) {
            return ws_server_->IsOnlyAudioClients();
        } else {
            return false;
        }
    }

    int WsPlugin::GetConnectedClientsCount() {
        if (IsWorking()) {
            return ws_server_->GetConnectedClientsCount();
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

    bool WsPlugin::HasEnoughBufferForQueuingMediaMessages() {
        return true;
    }

    bool WsPlugin::HasEnoughBufferForQueuingFtMessages() {
        return true;
    }

    bool WsPlugin::HasConnectedClients() {
        return GetConnectedClientsCount() > 0;
    }

    std::vector<std::shared_ptr<GrConnectedClientInfo>> WsPlugin::GetConnectedClientInfo() {
        if (IsWorking()) {
            return ws_server_->GetConnectedClientInfo();
        }
        return {};
    }

    void WsPlugin::DispatchAppEvent(const std::shared_ptr<AppBaseEvent> &event) {
        if (event->type_ == AppBaseEvent::EType::kClientHello) {
            auto target_event = std::dynamic_pointer_cast<MsgClientHello>(event);
            if (ws_server_) {
                ws_server_->OnClientHello(target_event);
            }
        }
    }

    GrNetPlugin* WsPlugin::GetLocalRtcPlugin() {
        if (auto plugin = GetPluginById(kNetRtcLocalPluginId); plugin) {
            return (GrNetPlugin*)plugin;
        }
        return nullptr;
    }

}

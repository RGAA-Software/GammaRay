//
// Created by RGAA on 21/11/2024.
//

#include "gr_net_plugin.h"
#include "gr_plugin_events.h"

namespace tc
{

    GrNetPlugin::GrNetPlugin() {
        plugin_type_ = GrPluginType::kNet;
    }

    GrNetPlugin::~GrNetPlugin() {

    }

    void GrNetPlugin::PostProtoMessage(const std::string& msg) {

    }

    bool GrNetPlugin::PostTargetStreamProtoMessage(const std::string& stream_id, const std::string& msg) {
        return false;
    }

    bool GrNetPlugin::PostTargetFileTransferProtoMessage(const std::string& stream_id, const std::string& msg) {
        return false;
    }

    void GrNetPlugin::OnClientEventCame(bool is_proto, int64_t socket_fd, const NetPluginType& nt_plugin_type, const std::string& msg) {
        auto event = std::make_shared<GrPluginNetClientEvent>();
        event->is_proto_ = is_proto;
        event->socket_fd_ = socket_fd;
        event->nt_plugin_type_ = nt_plugin_type;
        event->message_ = msg;
        CallbackEvent(event);
    }

    bool GrNetPlugin::IsOnlyAudioClients() {
        return false;
    }

    int GrNetPlugin::ConnectedClientSize() {
        return 0;
    }

    void GrNetPlugin::SyncInfo(const NetSyncInfo& info) {
        sync_info_ = info;
    }

    int64_t GrNetPlugin::GetQueuingMsgCount() {
        return 0;
    }

}

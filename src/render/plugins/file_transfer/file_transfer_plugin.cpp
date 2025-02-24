//
// Created RGAA on 15/11/2024.
//

#include "file_transfer_plugin.h"
#include "render/plugins/plugin_ids.h"
#include "render/plugin_interface/gr_net_plugin.h"
#include "tc_message.pb.h"

void* GetInstance() {
    static tc::FileTransferPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    std::string FileTransferPlugin::GetPluginId() {
        return kNetFileTransferPluginId;
    }

    std::string FileTransferPlugin::GetPluginName() {
        return "File Transfer Plugin";
    }

    std::string FileTransferPlugin::GetVersionName() {
        return "1.0.2";
    }

    uint32_t FileTransferPlugin::GetVersionCode() {
        return 102;
    }

    void FileTransferPlugin::OnMessage(const std::string& msg) {

    }

    void FileTransferPlugin::OnMessage(const std::shared_ptr<tc::Message>& msg) {
        auto type = msg->type();
        auto stream_id = msg->stream_id();
        PostToTargetStreamMessage(stream_id, msg->SerializeAsString());
    }
}

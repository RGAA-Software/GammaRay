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

    void FileTransferPlugin::OnProtoMessage(const std::string& msg) {
        // for testing
        auto m = new tc::Message();
        m->set_type(MessageType::kSyncPanelInfo);
        auto info = m->SerializeAsString();
        PostTargetStreamMessage("", info);
    }

    void FileTransferPlugin::PostTargetStreamMessage(const std::string& stream_id, const std::string& msg) {
        for (const auto& [plugin_id, plugin] : net_plugins_) {
            plugin->OnTargetStreamMessage(stream_id, msg);
        }
    }

}

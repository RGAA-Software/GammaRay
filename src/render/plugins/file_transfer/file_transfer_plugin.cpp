//
// Created RGAA on 15/11/2024.
//

#include "file_transfer_plugin.h"
#include "render/plugins/plugin_ids.h"
#include "render/plugin_interface/gr_net_plugin.h"
#include "tc_message.pb.h"
#include "file_transmission_server/file_transmit_msg_interface.h"

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
        return "File Transfer";
    }

    std::string FileTransferPlugin::GetVersionName() {
        return "1.0.2";
    }

    uint32_t FileTransferPlugin::GetVersionCode() {
        return 102;
    }

    std::string FileTransferPlugin::GetPluginDescription() {
        return "Full featured file transferring";
    }

    void FileTransferPlugin::OnMessage(const std::shared_ptr<tc::Message>& msg) {
        if (!file_trans_msg_interface_) {
            return;
        }
        file_trans_msg_interface_->OnMessage(msg);
    }

    bool FileTransferPlugin::OnCreate(const GrPluginParam& param) {
        if (!GrPluginInterface::OnCreate(param)) {
            return false;
        }
        file_trans_msg_interface_ = FileTransmitMsgInterface::Make(this);
        file_trans_msg_interface_->RegisterFileTransmitCallback();
        return true;
    };

}

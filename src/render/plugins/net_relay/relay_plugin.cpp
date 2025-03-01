//
// Created RGAA on 15/11/2024.
//

#include "relay_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "render/plugins/plugin_ids.h"
#include "tc_relay_client/relay_server_sdk.h"
#include "tc_relay_client/relay_server_sdk_param.h"
#include "relay_message.pb.h"

void* GetInstance() {
    static tc::RelayPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    std::string RelayPlugin::GetPluginId() {
        return kRelayPluginId;
    }

    std::string RelayPlugin::GetPluginName() {
        return "Relay Plugin";
    }

    std::string RelayPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t RelayPlugin::GetVersionCode() {
        return 110;
    }

    void RelayPlugin::On1Second() {
        GrPluginInterface::On1Second();

    }

    bool RelayPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrNetPlugin::OnCreate(param);

        // todo: debug
        device_id_ = "did_1011";
        // todo: check device id, empty? try to retry
        relay_sdk_ = std::make_shared<RelayServerSdk>(RelayServerSdkParam {
            .host_ = "127.0.0.1",
            .port_ = 20481,
            .ssl_ = false,
            .device_id_ = device_id_
        });

        relay_sdk_->SetOnRelayProtoMessageCallback([this](const std::shared_ptr<RelayMessage>& msg) {

        });

        relay_sdk_->Start();

        return true;
    }

    bool RelayPlugin::OnDestroy() {
        return true;
    }

    void RelayPlugin::PostProtoMessage(const std::string& msg) {
        relay_sdk_->RelayProtoMessage(msg);
    }

    bool RelayPlugin::PostTargetStreamProtoMessage(const std::string& stream_id, const std::string& msg) {
        // todo: stream id --> device id
        relay_sdk_->RelayProtoMessage(stream_id, msg);
        return true;
    }

    int RelayPlugin::ConnectedClientSize() {
        return 1;
    }

    bool RelayPlugin::IsOnlyAudioClients() {
        return false;
    }

    bool RelayPlugin::IsWorking() {
        return true;
    }

    void RelayPlugin::SyncInfo(const tc::NetSyncInfo &info) {

    }

}

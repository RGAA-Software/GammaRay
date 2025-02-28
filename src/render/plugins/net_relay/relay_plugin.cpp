//
// Created RGAA on 15/11/2024.
//

#include "relay_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "render/plugins/plugin_ids.h"
#include "tc_relay_client/relay_client_sdk.h"
#include "tc_relay_client/relay_client_sdk_param.h"

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

        relay_sdk_ = std::make_shared<RelayClientSdk>(RelayClientSdkParam {
            .host_ = "127.0.0.1",
            .port_ = 20481,
            .path_ = "/relay?device_id=xx0099",
            .ssl_ = false,
        });
        relay_sdk_->Start();

        return true;
    }

    bool RelayPlugin::OnDestroy() {
        return true;
    }

    void RelayPlugin::PostProtoMessage(const std::string &msg) {

    }

    bool RelayPlugin::PostTargetStreamProtoMessage(const std::string& stream_id, const std::string& msg) {

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

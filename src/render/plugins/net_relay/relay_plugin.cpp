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

        auto device_id = "server_" + sys_settings_.device_id_;
        auto relay_host = GetConfigParam<std::string>("relay_host");
        auto relay_port = GetConfigParam<std::string>("relay_port");

        LOGI("OnCreate, device id: {}, relay host: {}, relay port: {}", device_id, relay_host, relay_port);

        // todo: check device id, empty? try to retry
        relay_sdk_ = std::make_shared<RelayServerSdk>(RelayServerSdkParam {
            .host_ = relay_host,
            .port_ = std::atoi(relay_port.c_str()),
            .ssl_ = false,
            .device_id_ = device_id
        });

        relay_sdk_->SetOnRelayProtoMessageCallback([this](const std::shared_ptr<RelayMessage>& msg) {
            auto type = msg->type();
            if (type == RelayMessageType::kRelayTargetMessage) {
                auto sub = msg->relay();
                const auto& payload = sub.payload();
                auto msg = std::string(payload.data(), payload.size());
                this->OnClientEventCame(true, 0, NetPluginType::kWebSocket, msg);
            }
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

    void RelayPlugin::NotifyMediaClientConnected() {
        auto event = std::make_shared<GrPluginClientConnectedEvent>();
        this->CallbackEvent(event);
    }

    void RelayPlugin::NotifyMediaClientDisConnected() {
        auto event = std::make_shared<GrPluginClientDisConnectedEvent>();
        this->CallbackEvent(event);
    }
}

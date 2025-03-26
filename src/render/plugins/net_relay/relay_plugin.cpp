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

using namespace relay;

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

        std::thread([=, this]() {
            int connect_count = 0;
            for (;;) {
                auto device_id = "server_" + sys_settings_.device_id_;
                auto relay_host = GetConfigParam<std::string>("relay_host");
                auto relay_port = std::atoi(GetConfigParam<std::string>("relay_port").c_str());

                if (sys_settings_.device_id_.empty() || relay_host.empty() || relay_port <= 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }

                LOGI("OnCreate, connect count: {}; device id: {}, relay host: {}, relay port: {}",
                     connect_count++, device_id, relay_host, relay_port);

                // todo: check device id, empty? try to retry
                relay_sdk_ = std::make_shared<RelayServerSdk>(RelayServerSdkParam{
                    .host_ = relay_host,
                    .port_ = relay_port,
                    .ssl_ = false,
                    .device_id_ = device_id
                });

                relay_sdk_->SetOnConnectedCallback([=, this]() {
                    this->sdk_init_ = true;
                    this->NotifyMediaClientConnected();
                });

                relay_sdk_->SetOnDisConnectedCallback([=, this]() {
                    this->NotifyMediaClientDisConnected();
                });

                relay_sdk_->SetOnRoomPreparedCallback([this]() {
                    this->NotifyMediaClientConnected();
                });

                relay_sdk_->SetOnRoomDestroyedCallback([this]() {
                    this->NotifyMediaClientDisConnected();
                });

                relay_sdk_->SetOnRelayProtoMessageCallback([this](const std::shared_ptr<RelayMessage> &msg) {
                    auto type = msg->type();
                    if (type == RelayMessageType::kRelayTargetMessage) {
                        auto sub = msg->relay();
                        const auto &payload = sub.payload();
                        auto msg = std::string(payload.data(), payload.size());
                        this->OnClientEventCame(true, 0, NetPluginType::kWebSocket, msg);
                    }
                });

                relay_sdk_->Start();

                std::this_thread::sleep_for(std::chrono::seconds(2));
                if (this->sdk_init_) {
                    LOGI("SDK Connected to server!");
                    break;
                }
                else {
                    LOGI("Will retry to connect relay server.");
                    relay_sdk_->Stop();
                }
            }
        }).detach();

        return true;
    }

    bool RelayPlugin::OnDestroy() {
        return true;
    }

    void RelayPlugin::PostProtoMessage(const std::string& msg) {
        if (IsWorking()) {
            relay_sdk_->RelayProtoMessage(msg);
        }
    }

    bool RelayPlugin::PostTargetStreamProtoMessage(const std::string& stream_id, const std::string& msg) {
        // todo: stream id --> device id
        if (IsWorking()) {
            relay_sdk_->RelayProtoMessage(stream_id, msg);
        }
        return true;
    }

    int RelayPlugin::ConnectedClientSize() {
        return 1;
    }

    bool RelayPlugin::IsOnlyAudioClients() {
        return false;
    }

    bool RelayPlugin::IsWorking() {
        return sdk_init_ && relay_sdk_;
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

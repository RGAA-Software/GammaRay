//
// Created RGAA on 15/11/2024.
//

#include "relay_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "tc_common_new/ip_util.h"
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
            auto ips = IPUtil::ScanIPs();
            std::vector<RelayDeviceNetInfo> net_info_;
            for (const auto& info : ips) {
                net_info_.push_back(RelayDeviceNetInfo {
                    .ip_ = info.ip_addr_,
                    .mac_ = info.mac_address_,
                });
            }

            for (;;) {
                auto srv_device_id = "server_" + sys_settings_.device_id_;
                auto relay_host = GetConfigParam<std::string>("relay_host");
                auto relay_port = std::atoi(GetConfigParam<std::string>("relay_port").c_str());

                if (relay_host != sys_settings_.relay_host_ && !sys_settings_.relay_host_.empty()) {
                    relay_host = sys_settings_.relay_host_;
                }

                auto sys_relay_port = std::atoi(sys_settings_.relay_port_.c_str());
                if (relay_port != sys_relay_port && sys_relay_port > 0) {
                    relay_port = sys_relay_port;
                }
                LOGI("OnCreate try to connect, connect count: {}; device id: {}, relay host: {}, relay port: {}",
                     connect_count++, srv_device_id, relay_host, relay_port);

                if (sys_settings_.device_id_.empty() || relay_host.empty() || relay_port <= 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }

                // todo: check device id, empty? try to retry
                relay_media_sdk_ = std::make_shared<RelayServerSdk>(RelayServerSdkParam{
                    .host_ = relay_host,
                    .port_ = relay_port,
                    .ssl_ = false,
                    .device_id_ = srv_device_id,
                    .net_info_ = net_info_,
                });

                relay_media_sdk_->SetOnConnectedCallback([=, this]() {
                    this->sdk_init_ = true;
                    this->NotifyMediaClientConnected();
                });

                relay_media_sdk_->SetOnDisConnectedCallback([=, this]() {
                    this->NotifyMediaClientDisConnected();
                });

                relay_media_sdk_->SetOnRoomPreparedCallback([this]() {
                    this->NotifyMediaClientConnected();
                });

                relay_media_sdk_->SetOnRoomDestroyedCallback([this]() {
                    this->NotifyMediaClientDisConnected();
                });

                relay_media_sdk_->SetOnRelayProtoMessageCallback([this](const std::shared_ptr<RelayMessage> &msg) {
                    auto type = msg->type();
                    if (type == RelayMessageType::kRelayTargetMessage) {
                        auto sub = msg->relay();
                        auto relay_msg_index = sub.relay_msg_index();
                        const auto &payload = sub.payload();
                        auto payload_msg = std::string(payload.data(), payload.size());
                        this->OnClientEventCame(true, 0, NetPluginType::kWebSocket, payload_msg);
                    }
                    else if (type == RelayMessageType::kRelayRequestPausedStream) {
                        paused_stream = true;

                        auto event = std::make_shared<GrPluginRelayPausedEvent>();
                        this->CallbackEvent(event);

                    }
                    else if (type == RelayMessageType::kRelayRequestResumeStream) {
                        paused_stream = false;

                        auto event = std::make_shared<GrPluginRelayResumedEvent>();
                        this->CallbackEvent(event);
                    }
                });

                relay_media_sdk_->Start();

                std::this_thread::sleep_for(std::chrono::seconds(2));
                if (this->sdk_init_) {
                    LOGI("SDK Connected to server, connect file transfer channel");

                    auto ft_device_id = "ft_server_" + sys_settings_.device_id_;
                    relay_ft_sdk_ = std::make_shared<RelayServerSdk>(RelayServerSdkParam{
                        .host_ = relay_host,
                        .port_ = relay_port,
                        .ssl_ = false,
                        .device_id_ = ft_device_id,
                        .net_info_ = net_info_,
                    });

                    relay_ft_sdk_->SetOnRelayProtoMessageCallback([this](const std::shared_ptr<RelayMessage> &msg) {
                        auto type = msg->type();
                        if (type == RelayMessageType::kRelayTargetMessage) {
                            auto sub = msg->relay();
                            auto relay_msg_index = sub.relay_msg_index();
                            if (recv_relay_ft_msg_index_ == 0) {
                                recv_relay_ft_msg_index_ = relay_msg_index;
                            }
                            else {
                                auto diff = relay_msg_index - recv_relay_ft_msg_index_;
                                if (diff != 1) {
                                    LOGE("FT error sequence, current: {}, last: {}", relay_msg_index, recv_relay_ft_msg_index_);
                                }
                                recv_relay_ft_msg_index_ = relay_msg_index;
                            }
                            const auto &payload = sub.payload();
                            auto payload_msg = std::string(payload.data(), payload.size());
                            this->OnClientEventCame(true, 0, NetPluginType::kWebSocket, payload_msg);
                        }
                    });

                    relay_ft_sdk_->Start();

                    break;
                }
                else {
                    LOGI("Will retry to connect relay server.");
                    relay_media_sdk_->Stop();
                    if (relay_media_sdk_) {
                        relay_media_sdk_->Stop();
                    }
                }
            }
        }).detach();

        return true;
    }

    bool RelayPlugin::OnDestroy() {
        return true;
    }

    void RelayPlugin::PostProtoMessage(const std::string& msg) {
        if (IsWorking() && !paused_stream) {
            relay_media_sdk_->RelayProtoMessage(msg);
        }
    }

    bool RelayPlugin::PostTargetStreamProtoMessage(const std::string& stream_id, const std::string& msg) {
        // todo: stream id --> device id
        if (IsWorking() && !paused_stream) {
            relay_media_sdk_->RelayProtoMessage(stream_id, msg);
        }
        return true;
    }

    bool RelayPlugin::PostTargetFileTransferProtoMessage(const std::string &stream_id, const std::string &msg) {
        if (IsWorking() && relay_ft_sdk_ && !paused_stream) {
            relay_ft_sdk_->RelayProtoMessage(stream_id, msg);
        }
        return true;
    }

    int RelayPlugin::ConnectedClientSize() {
        return IsWorking() ? 1 : 0;
    }

    bool RelayPlugin::IsOnlyAudioClients() {
        return false;
    }

    bool RelayPlugin::IsWorking() {
        return sdk_init_ && relay_media_sdk_;
    }

    void RelayPlugin::SyncInfo(const tc::NetSyncInfo &info) {
        GrNetPlugin::SyncInfo(info);
    }

    void RelayPlugin::NotifyMediaClientConnected() {
        auto event = std::make_shared<GrPluginClientConnectedEvent>();
        this->CallbackEvent(event);
    }

    void RelayPlugin::NotifyMediaClientDisConnected() {
        auto event = std::make_shared<GrPluginClientDisConnectedEvent>();
        this->CallbackEvent(event);
    }

    void RelayPlugin::OnSyncSystemSettings(const tc::GrPluginSettingsInfo &settings) {
        GrPluginInterface::OnSyncSystemSettings(settings);
    }

    int64_t RelayPlugin::GetQueuingMediaMsgCount() {
        return relay_media_sdk_ && sdk_init_ ? relay_media_sdk_->GetQueuingMsgCount() : 0;
    }

    int64_t RelayPlugin::GetQueuingFtMsgCount() {
        return relay_ft_sdk_ && sdk_init_ ? relay_ft_sdk_->GetQueuingMsgCount() : 0;
    }
}

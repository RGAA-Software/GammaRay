//
// Created by RGAA on 17/05/2025.
//

#include "ct_spvr_client.h"
#include "tc_common_new/log.h"
#include "ct_client_context.h"
#include "spvr_client.pb.h"
#include "thunder_sdk.h"
#include "tc_common_new/time_util.h"

namespace tc
{
    CtSpvrClient::CtSpvrClient(const std::shared_ptr<ThunderSdk>& sdk,
                               const std::shared_ptr<ClientContext>& ctx,
                               const std::string& host,
                               int port,
                               const std::string& device_id,
                               const std::string& remote_device_id,
                               const std::string& appkey) {
        sdk_ = sdk;
        context_ = ctx;
        host_ = host;
        port_ = port;
        device_id_ = device_id;
        remote_device_id_ = remote_device_id;
        appkey_ = appkey;
    }

    void CtSpvrClient::Start() {
        client_ = std::make_shared<asio2::wss_client>();
        client_->set_auto_reconnect(true);
        client_->keep_alive(true);
        client_->set_timeout(std::chrono::milliseconds(3000));
        client_->set_verify_mode(asio::ssl::verify_none);

        client_->start_timer("spvr_client_hb", std::chrono::seconds(1), [=, this]() {
            if (!client_->is_started()) {
                return;
            }
            this->Heartbeat();
        });

        client_->bind_init([&]() {
            client_->ws_stream().binary(true);
            client_->set_no_delay(true);

        })
        .bind_connect([=, this]() {
            if (asio2::get_last_error()) {
                LOGE("connect failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
            } else {
                LOGI("connect success : {} {} ", client_->local_address().c_str(), client_->local_port());
            }

            client_->post_queued_event([=, this]() {
                this->Hello();
            });

        })
        .bind_upgrade([=, this]() {
            if (asio2::get_last_error()) {
                LOGE("upgrade failure : {}, {}", asio2::last_error_val(), asio2::last_error_msg());
            }
        })
        .bind_disconnect([=, this]() {
            LOGE("*** Disconnected for spvr-client: {}", device_id_);
        })
        .bind_recv([=, this](std::string_view data) {
            auto msg = std::string(data.data(), data.size());
            this->ParseMessage(msg);
        });

        // the /ws is the websocket upgraged target
        auto path = std::format("/spvr/client?appkey={}&device_id={}&remote_device_id={}", appkey_, device_id_, remote_device_id_);
        LOGI("will connect => {}:{}{}", host_, port_, path);
        if (!client_->async_start(host_, port_, path)) {
            LOGE("connect websocket server failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }
    }

    void CtSpvrClient::Exit() const {
        if (client_) {
            client_->stop_all_timers();
            client_->stop();
        }
    }

    bool CtSpvrClient::IsAlive() const {
        return client_ && client_->is_started();
    }

    void CtSpvrClient::Hello() {
        if (!IsAlive()) {
            return;
        }
        spvr_client::SpvrClientMessage msg;
        msg.set_msg_type(spvr_client::SpvrClientMessageType::kSpvrClientHello);
        msg.set_device_id(device_id_);
        const auto sub = msg.mutable_hello();
        sub->set_device_id(device_id_);
        client_->async_send(msg.SerializeAsString());
    }


    void CtSpvrClient::Heartbeat() {
        if (!IsAlive()) {
            return;
        }
        auto sdk_last_hb_ts = sdk_->GetLastHeartbeatTimestamp();
        bool alive = (TimeUtil::GetCurrentTimestamp() - sdk_last_hb_ts) < 10'000;
        spvr_client::SpvrClientMessage msg;
        msg.set_msg_type(spvr_client::SpvrClientMessageType::kSpvrClientHeartBeat);
        msg.set_device_id(device_id_);
        const auto sub = msg.mutable_heartbeat();
        sub->set_hb_index(hb_index_++);
        sub->set_connection_alive(alive);
        client_->async_send(msg.SerializeAsString());
    }

    void CtSpvrClient::ParseMessage(const std::string& data) {
        auto msg = std::make_shared<spvr_client::SpvrClientMessage>();
        if (!msg->ParseFromArray(data.data(), data.size())) {
            LOGE("CtSpvrClient parse message failed!");
            return;
        }
        if (msg->msg_type() == spvr_client::SpvrClientMessageType::kSpvrClientHeartBeat) {
            LOGI("Heartbeat: {}", msg->device_id(), msg->heartbeat().hb_index());
        }
    }

}

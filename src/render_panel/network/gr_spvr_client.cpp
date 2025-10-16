//
// Created by RGAA on 17/05/2025.
//

#include "gr_spvr_client.h"
#include "spvr_panel.pb.h"
#include "tc_common_new/log.h"
#include "render_panel/gr_context.h"
#include "tc_common_new/message_notifier.h"
#include "render_panel/gr_app_messages.h"

namespace tc
{
    GrSpvrClient::GrSpvrClient(const std::shared_ptr<GrContext>& ctx,
                               const std::string& host,
                               int port,
                               const std::string& device_id,
                               const std::string& appkey) {
        context_ = ctx;
        host_ = host;
        port_ = port;
        device_id_ = device_id;
        appkey_ = appkey;

        msg_listener_ = context_->ObtainMessageListener();
        msg_listener_->Listen<MsgGrTimer2S>([=, this](const MsgGrTimer2S& m) {
            context_->PostTask([=, this]() {
                this->Heartbeat();
            });
        });

    }

    void GrSpvrClient::Start() {
        client_ = std::make_shared<asio2::wss_client>();
        client_->set_auto_reconnect(true);
        client_->keep_alive(true);
        client_->set_timeout(std::chrono::milliseconds(3000));
        client_->set_verify_mode(asio::ssl::verify_none);

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
        auto path = std::format("/spvr/panel?appkey={}&device_id={}", appkey_, device_id_);
        LOGI("will connect => {}:{}{}", host_, port_, path);
        if (!client_->async_start(host_, port_, path)) {
            LOGE("connect websocket server failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }
    }

    bool GrSpvrClient::IsStarted() {
        return client_ != nullptr;
    }

    bool GrSpvrClient::IsActive() {
        return IsStarted() && client_->is_started();
    }

    void GrSpvrClient::Hello() {
        if (!IsActive()) {
            return;
        }
        spvr_panel::SpvrPanelMessage msg;
        msg.set_msg_type(spvr_panel::SpvrPanelMessageType::kSpvrPanelHello);
        auto sub = msg.mutable_hello();
        sub->set_device_id(device_id_);
        PostBinMessage(msg.SerializeAsString());
    }

    void GrSpvrClient::Heartbeat() {
        if (!IsActive()) {
            return;
        }
        spvr_panel::SpvrPanelMessage msg;
        msg.set_msg_type(spvr_panel::SpvrPanelMessageType::kSpvrPanelHeartBeat);
        auto sub = msg.mutable_heartbeat();
        sub->set_hb_index(hb_idx_++);
        PostBinMessage(msg.SerializeAsString());
    }

    void GrSpvrClient::PostBinMessage(const std::string& m) {
        if (IsActive()) {
            client_->async_send(m);
        }
    }

    void GrSpvrClient::ParseMessage(std::string_view data) {

    }

}
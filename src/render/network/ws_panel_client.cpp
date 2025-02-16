//
// Created by RGAA on 2024-04-20.
//

#include "ws_panel_client.h"
#include "context.h"
#include "render/app/app_messages.h"
#include "render/statistics.h"
#include "render/settings/settings.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include "tc_message.pb.h"

namespace tc
{

    const int kMaxClientQueuedMessage = 1024;

    WsPanelClient::WsPanelClient(const std::shared_ptr<Context>& ctx) {
        statistics_ = Statistics::Instance();
        settings_ = Settings::Instance();
        context_ = ctx;
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgTimer100>([=, this](const MsgTimer100& msg) {
            this->SendStatistics();
        });
    }

    void WsPanelClient::Start() {
        client_ = std::make_shared<asio2::ws_client>();
        client_->set_auto_reconnect(true);
        client_->set_timeout(std::chrono::milliseconds(2000));

        client_->bind_init([&]() {
            client_->ws_stream().binary(true);
            client_->set_no_delay(true);
            client_->ws_stream().set_option(
                websocket::stream_base::decorator([](websocket::request_type &req) {
                    req.set(http::field::authorization, "websocket-client-authorization");}
                )
            );

        }).bind_connect([&]() {
            if (asio2::get_last_error()) {
                LOGE("connect failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
            } else {
                LOGI("connect success : {} {} ", client_->local_address().c_str(), client_->local_port());
            }
        }).bind_upgrade([&]() {
            if (asio2::get_last_error()) {
                LOGE("upgrade failure : {}, {}", asio2::last_error_val(), asio2::last_error_msg());
            }
        }).bind_recv([this](std::string_view data) {
            ParseNetMessage(data);
        });

        if (!client_->async_start("127.0.0.1", settings_->panel_server_port_, "/panel")) {
            LOGE("connect websocket server failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }
    }

    void WsPanelClient::Exit() {
        if (client_) {
            client_->stop();
        }
    }

    void WsPanelClient::SendStatistics() {
        PostNetMessage(statistics_->AsProtoMessage());
    }

    void WsPanelClient::PostNetMessage(const std::string& msg) {
        if (client_ && client_->is_started()) {
            if (queued_msg_count_ > kMaxClientQueuedMessage) {
                return;
            }
            queued_msg_count_++;
            client_->async_send(msg, [=, this]() {
                queued_msg_count_--;
            });
        }
    }

    void WsPanelClient::ParseNetMessage(std::string_view _msg) {
        try {
            std::string msg = std::string(_msg);
            tc::Message m;
            m.ParseFromString(msg);
            if (m.type() == MessageType::kSyncPanelInfo) {
                auto sub = m.sync_panel_info();
                settings_->client_id_ = sub.client_id();
                settings_->client_random_pwd_ = sub.client_random_pwd();
                //LOGI("SyncPanelInfo, client id: {}, random pwd: {}", sub.client_id(), sub.client_random_pwd());
            }
        } catch(std::exception& e) {
            LOGE("ParseNetMessage failed: {}", e.what());
        }
    }

}
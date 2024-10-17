//
// Created by RGAA on 2024-04-20.
//

#include "ws_client.h"
#include "context.h"
#include "server/app/app_messages.h"
#include "server/statistics.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"

namespace tc
{

    const int kMaxClientQueuedMessage = 4096;

    WSClient::WSClient(const std::shared_ptr<Context>& ctx) {
        statistics_ = Statistics::Instance();
        context_ = ctx;
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgTimer100>([=, this](const MsgTimer100& msg) {
            this->SendStatistics();
        });
    }

    void WSClient::Start() {
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
        }).bind_recv([&](std::string_view data) {

        });

        // the /ws is the websocket upgraged target
        if (!client_->async_start("127.0.0.1", 20369, "/")) {
            LOGE("connect websocket server failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }
    }

    void WSClient::Exit() {
        if (client_) {
            client_->stop();
        }
    }

    void WSClient::SendStatistics() {
        PostNetMessage(statistics_->AsProtoMessage());
    }

    void WSClient::PostNetMessage(const std::string& msg) {
        if (client_ && client_->is_started()) {
            if (queued_msg_count_ > kMaxClientQueuedMessage) {
                LOGW("too many message in queue, discard the message in WSClient");
                return;
            }
            queued_msg_count_++;
            client_->async_send(msg, [=, this]() {
                queued_msg_count_--;
            });
        }
    }

}
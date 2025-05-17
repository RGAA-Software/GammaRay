//
// Created by RGAA on 17/05/2025.
//

#include "ct_dashboard_client.h"
#include "tc_common_new/log.h"
#include "ct_client_context.h"

namespace tc
{
    CtDashboardClient::CtDashboardClient(const std::shared_ptr<ClientContext>& ctx) {
        context_ = ctx;
    }

    void CtDashboardClient::Start() {
        client_ = std::make_shared<asio2::ws_client>();
        client_->set_auto_reconnect(true);
        client_->keep_alive(true);
        client_->set_timeout(std::chrono::milliseconds(3000));

        client_->bind_init([&]() {
            client_->ws_stream().binary(true);
            client_->set_no_delay(true);

        }).bind_connect([&]() {
            if (asio2::get_last_error()) {
                LOGE("connect failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
            } else {
                LOGI("connect success : {} {} ", client_->local_address().c_str(), client_->local_port());
            }

            context_->PostTask([=, this]() {
                //context_->SendAppMessage(MsgConnectedToService{});
            });

        }).bind_upgrade([&]() {
            if (asio2::get_last_error()) {
                LOGE("upgrade failure : {}, {}", asio2::last_error_val(), asio2::last_error_msg());
            }
        }).bind_recv([=, this](std::string_view data) {
            auto msg = std::string(data.data(), data.size());
            this->ParseMessage(msg);
        });

        // the /ws is the websocket upgraged target
        if (!client_->async_start("127.0.0.1", 20375, "/service/message?from=panel")) {
            LOGE("connect websocket server failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }
    }

    void CtDashboardClient::ParseMessage(std::string_view data) {

    }
}
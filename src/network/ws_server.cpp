//
// Created by RGAA on 2024-03-30.
//

#include "ws_server.h"
#include "gr_settings.h"
#include "tc_common_new/log.h"
#include "tc_message.pb.h"
#include "gr_context.h"
#include "app_messages.h"

namespace tc
{

    std::shared_ptr<WSServer> WSServer::Make(const std::shared_ptr<GrContext> &ctx) {
        return std::make_shared<WSServer>(ctx);
    }

    WSServer::WSServer(const std::shared_ptr<GrContext> &ctx) {
        context_ = ctx;
    }

    void WSServer::Start() {
        server_ = std::make_shared<asio2::ws_server>();
        server_->bind_accept([&](std::shared_ptr<asio2::ws_session> &session_ptr) {
            // accept callback maybe has error like "Too many open files", etc...
            if (!asio2::get_last_error()) {
                // Set the binary message write option.
                session_ptr->ws_stream().binary(true);

                // Set the text message write option. The sent text must be utf8 format.
                //session_ptr->ws_stream().text(true);

                // how to set custom websocket response data :
                // the decorator is just a callback function, when the upgrade response is send,
                // this callback will be called.
                session_ptr->ws_stream().set_option(
                        websocket::stream_base::decorator([session_ptr](websocket::response_type &rep) {
                            // @see /asio2/example/websocket/client/websocket_client.cpp
                            const websocket::request_type &req = session_ptr->get_upgrade_request();
                            auto it = req.find(http::field::authorization);
                            if (it != req.end())
                                rep.set(http::field::authentication_results, "200 OK");
                            else
                                rep.set(http::field::authentication_results, "401 unauthorized");
                        }));
            } else {
                printf("error occurred when calling the accept function : %d %s\n",
                       asio2::get_last_error_val(), asio2::get_last_error_msg().data());
            }
        }).bind_recv([&](auto &session_ptr, std::string_view data) {
            this->ParseBinaryMessage(data);

        }).bind_connect([](auto &session_ptr) {
//            printf("client enter : %s %u %s %u\n",
//                   session_ptr->remote_address().c_str(), session_ptr->remote_port(),
//                   session_ptr->local_address().c_str(), session_ptr->local_port());

        }).bind_disconnect([](auto &session_ptr) {
            asio2::ignore_unused(session_ptr);
            printf("client leave : %s\n", asio2::last_error_msg().c_str());

        }).bind_upgrade([](auto &session_ptr) {
            printf("client upgrade : %s %u %d %s\n",
                   session_ptr->remote_address().c_str(), session_ptr->remote_port(),
                   asio2::last_error_val(), asio2::last_error_msg().c_str());

            // how to get the upgrade request data :
            // @see /asio2/example/websocket/client/websocket_client.cpp
            const websocket::request_type &req = session_ptr->get_upgrade_request();
            auto it = req.find(http::field::authorization);
            if (it != req.end()) {
                beast::string_view auth = it->value();
                std::cout << auth << std::endl;
                        ASIO2_ASSERT(auth == "websocket-client-authorization");
            }

        }).bind_start([&]() {
            if (asio2::get_last_error()) {
                LOGE("start websocket server failure : %s %u %d %s\n",
                       server_->listen_address().c_str(), server_->listen_port(),
                       asio2::last_error_val(), asio2::last_error_msg().c_str());
            }
        }).bind_stop([&]() {

        });

        server_->start("0.0.0.0", GrSettings::Instance()->ws_server_port_);
    }

    void WSServer::Exit() {

    }

    WSServer::~WSServer() {
        if (server_) {
            server_->stop();
        }
    }

    void WSServer::ParseBinaryMessage(std::string_view msg) {
        auto proto_msg = std::make_shared<tc::Message>();
        if (!proto_msg->ParseFromArray(msg.data(), msg.size())) {
            LOGE("Parse binary message failed.");
            return;
        }
        auto capture_statistics = proto_msg->capture_statistics();
        context_->SendAppMessage(MsgCaptureStatistics{
                .msg_ = proto_msg,
                .statistics_ = capture_statistics,
        });
    }

}
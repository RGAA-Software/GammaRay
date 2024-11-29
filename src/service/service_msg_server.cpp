//
// Created by RGAA on 29/11/2024.
//

#include "service_msg_server.h"
#include "tc_common_new/log.h"
#include "render_manager.h"
#include "service.h"

namespace tc
{
    ServiceMsgServer::ServiceMsgServer(const std::shared_ptr<ServiceContext>& context) {
        context_ = context;
    }

    void ServiceMsgServer::Init(const std::shared_ptr<GrService>& service) {
        service_ = service;
    }

    void ServiceMsgServer::Start() {
        server_ = std::make_shared<asio2::ws_server>();
        server_->bind_accept([=, this](std::shared_ptr<asio2::ws_session> &session_ptr) {
            if (!asio2::get_last_error()) {
                session_ = session_ptr;
                session_ptr->ws_stream().binary(true);
                session_ptr->ws_stream().set_option(
                        websocket::stream_base::decorator([session_ptr](websocket::response_type &rep) {
                            const websocket::request_type &req = session_ptr->get_upgrade_request();
                            auto it = req.find(http::field::authorization);
                            if (it != req.end())
                                rep.set(http::field::authentication_results, "200 OK");
                            else
                                rep.set(http::field::authentication_results, "401 unauthorized");
                        }));
            } else {
                LOGI("error occurred when calling the accept function : {} {}",
                       asio2::get_last_error_val(), asio2::get_last_error_msg().data());
            }
        }).bind_recv([=, this](auto &session_ptr, std::string_view data) {
            this->ParseMessage(data);
        }).bind_connect([](auto &session_ptr) {

        }).bind_disconnect([=](auto &session_ptr) {

        }).bind_upgrade([](auto &session_ptr) {

        }).bind_start([&]() {

        }).bind_stop([&]() {

        });

        server_->start("0.0.0.0", 20375, "/service/message");
    }

    void ServiceMsgServer::ParseMessage(std::string_view data) {
        LOGI("data: {}", data);
        auto resp = std::format("you sent: {}", data);
        session_->async_send(resp);

        if (data == "start") {
            auto render_mgr = service_->GetRenderManager();
            render_mgr->StartServer();
        }
    }

    void ServiceMsgServer::PostBinaryMessage(const std::string& msg) {
        if (session_ && server_ && server_->is_started()) {
            session_->async_send(msg);
        }
    }
}
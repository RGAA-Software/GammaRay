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
            if (!asio2::get_last_error()) {
                session_ptr->ws_stream().binary(true);
                session_ptr->set_no_delay(true);
            } else {
                LOGE("error occurred when calling the accept function, err: {}, msg: {}",
                     asio2::get_last_error_val(), asio2::get_last_error_msg().data());
            }
        }).bind_recv([&](auto& sess, std::string_view data) {
            auto socket_fd = (uint64_t)sess->socket().native_handle();
            this->ParseBinaryMessage(socket_fd, data);

        }).bind_connect([=, this](std::shared_ptr<asio2::ws_session>& sess) {
            auto socket_fd = (uint64_t)sess->socket().native_handle();
            auto ws_sess = std::make_shared<WSSession>();
            ws_sess->socket_fd_ = socket_fd;
            ws_sess->session_ = sess;
            this->sessions_.Insert(socket_fd, ws_sess);
            LOGI("client connect : {}", socket_fd);

        }).bind_disconnect([=, this](auto &sess) {
            auto socket_fd = (uint64_t)sess->socket().native_handle();
            this->sessions_.Remove(socket_fd);
            LOGI("client leave : {}", socket_fd);

        }).bind_upgrade([](auto &session_ptr) {

        }).bind_start([&]() {
            if (asio2::get_last_error()) {
                LOGE("start websocket server failure, address: {}, listen port: {}, error val: {}, error msg: {}",
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

    void WSServer::PostBinaryMessage(const std::string& msg, bool only_inner) {
        sessions_.VisitAll([=, this](uint64_t fd, std::shared_ptr<WSSession>& sess) {
            if (only_inner && sess->session_type_ != tc::SessionType::kInnerServer) {
                return;
            }
            if (sess->session_) {
                sess->session_->async_send(msg);
            }
        });
    }

    void WSServer::PostBinaryMessage(std::string_view msg, bool only_inner) {
        sessions_.VisitAll([=, this](uint64_t fd, std::shared_ptr<WSSession>& sess) {
            if (only_inner && sess->session_type_ != tc::SessionType::kInnerServer) {
                return;
            }
            if (sess->session_) {
                sess->session_->async_send(msg);
            }
        });
    }

    void WSServer::ParseBinaryMessage(uint64_t socket_fd, std::string_view msg) {
        auto proto_msg = std::make_shared<tc::Message>();
        if (!proto_msg->ParseFromArray(msg.data(), msg.size())) {
            LOGE("Parse binary message failed.");
            return;
        }
        if (proto_msg->type() == tc::kUIServerHello) {
            auto hello = proto_msg->ui_server_hello();
            sessions_.VisitAll([=](uint64_t k, std::shared_ptr<WSSession>& v) {
                if (v->socket_fd_ == socket_fd) {
                    v->session_type_ = hello.type();
                    LOGI("Update session type: {} for socket: {}", v->session_type_, socket_fd);
                }
            });

        } else if (proto_msg->type() == tc::kCaptureStatistics) {
            auto capture_statistics = proto_msg->capture_statistics();
            context_->SendAppMessage(MsgCaptureStatistics{
                .msg_ = proto_msg,
                .statistics_ = capture_statistics,
            });

        } else if (proto_msg->type() == tc::kServerAudioSpectrum) {
            auto spectrum = proto_msg->server_audio_spectrum();
            context_->SendAppMessage(MsgServerAudioSpectrum {
                .msg_ = proto_msg,
                .spectrum_ = spectrum,
            });
        }
    }

}
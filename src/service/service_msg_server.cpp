//
// Created by RGAA on 29/11/2024.
//

#include "service_msg_server.h"
#include "tc_common_new/log.h"
#include "render_manager.h"
#include "service.h"
#include "tc_common_new/url_helper.h"
#include "tc_service_message.pb.h"

namespace tc
{
    ServiceMsgServer::ServiceMsgServer(const std::shared_ptr<ServiceContext>& context, const std::shared_ptr<RenderManager>& rm) {
        context_ = context;
        render_manager_ = rm;
    }

    void ServiceMsgServer::Init(const std::shared_ptr<GrService>& service) {
        service_ = service;
    }

    void ServiceMsgServer::Start() {
        ws_server_ = std::make_shared<asio2::ws_server>();

        auto fn_get_socket_fd = [](std::shared_ptr<asio2::ws_session> &sess_ptr) -> uint64_t {
            auto& s = sess_ptr->socket();
            return (uint64_t)s.native_handle();
        };

        ws_server_->bind_accept([&](std::shared_ptr<asio2::ws_session>& session_ptr) {
           // accept callback maybe has error like "Too many open files", etc...
           if (!asio2::get_last_error()) {
               // Set the binary message write option.
               session_ptr->ws_stream().binary(true);

               // how to set custom websocket response data :
               // the decorator is just a callback function, when the upgrade response is send,
               // this callback will be called.
               session_ptr->ws_stream().set_option(
                   websocket::stream_base::decorator([session_ptr](websocket::response_type& rep) {
                    // @see /asio2/example/websocket/client/websocket_client.cpp
                    const websocket::request_type& req = session_ptr->get_upgrade_request();
                    auto it = req.find(http::field::authorization);
                    if (it != req.end())
                        rep.set(http::field::authentication_results, "200 OK");
                    else
                        rep.set(http::field::authentication_results, "401 unauthorized");
                    }));
           }
           else {
               LOGE("error occurred when calling the accept function : {} {}",
                      asio2::get_last_error_val(), asio2::get_last_error_msg().data());
           }
       })
       .bind_recv([=, this](auto& sess_ptr, std::string_view data) {
           auto socket_fd = fn_get_socket_fd(sess_ptr);
           auto sw = [&]() -> std::shared_ptr<SessionWrapper> {
               if (sessions_.HasKey(socket_fd)) {
                   return sessions_.Get(socket_fd);
               }
               else {
                    return nullptr;
               }
           } ();
           if (sw) {
               this->ParseMessage(sw, data);
           }
       })
       .bind_connect([=, this](std::shared_ptr<asio2::ws_session>& sess_ptr) {
           sess_ptr->set_disconnect_timeout((std::chrono::steady_clock::duration::max)());
           sess_ptr->set_no_delay(true);
           auto socket_fd = fn_get_socket_fd(sess_ptr);
           auto sess_wrapper = std::make_shared<SessionWrapper>();
           sess_wrapper->socket_fd_ = socket_fd;
           sess_wrapper->session_ = sess_ptr;
           sessions_.Insert(socket_fd, sess_wrapper);
       })
       .bind_disconnect([=, this](auto& sess_ptr) {
           auto socket_fd = fn_get_socket_fd(sess_ptr);
           LOGI("client closed: {}", socket_fd);
           sessions_.Remove(socket_fd);
       });

        bool ret = ws_server_->start("0.0.0.0", 20375);
        LOGI("service start at: 0.0.0.0:20375/service/message, result: {}", ret);
    }

    void ServiceMsgServer::ParseMessage(const std::shared_ptr<SessionWrapper>& sw, std::string_view data) {
        tc::ServiceMessage msg;
        try {
            msg.ParseFromString(std::string(data.data(), data.size()));
            auto type = msg.type();
            if (type == ServiceMessageType::kStartServer) {
                auto sub = msg.start_server();
                std::vector<std::string> args;
                for (int i = 0; i < sub.args_size(); i++) {
                    args.push_back(sub.args(i));
                }
                ProcessStartRender(sub.work_dir(), sub.app_path(), args);
            }
            else if (type == ServiceMessageType::kStopServer) {
                ProcessStopRender();
            }
            else if (type == ServiceMessageType::kRestartServer) {
                auto sub = msg.restart_server();
                std::vector<std::string> args;
                for (int i = 0; i < sub.args_size(); i++) {
                    args.push_back(sub.args(i));
                }
                ProcessRestartRender(sub.work_dir(), sub.app_path(), args);
            }
            else if (type == ServiceMessageType::kHeartBeat) {
                auto sub = msg.heart_beat();
                sw->from_ = sub.from();
                ProcessHeartBeat(sub.index());
            }
        }
        catch(...) {
            LOGE("Parse message failed!");
        }
    }

    void ServiceMsgServer::ProcessStartRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args) {
        if (!render_manager_->StartServer(work_dir, app_path, args)) {
            LOGE("Start server failed!");
        }
    }

    void ServiceMsgServer::ProcessStopRender() {
        render_manager_->StopServer();
    }

    void ServiceMsgServer::ProcessRestartRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args) {
        render_manager_->ReStart(work_dir, app_path, args);
    }

    void ServiceMsgServer::ProcessHeartBeat(int64_t index) {
        auto is_render_alive = render_manager_->IsRenderAlive();
        ServiceMessage msg;
        msg.set_type(ServiceMessageType::kHeartBeatResp);
        auto sub = msg.mutable_heart_beat_resp();
        sub->set_index(index);
        sub->set_render_status(is_render_alive ? RenderStatus::kWorking : RenderStatus::kStopped);
        PostBinaryMessage(msg.SerializeAsString());
    }

    void ServiceMsgServer::PostBinaryMessage(const std::string& msg) {
        if (ws_server_ && ws_server_->is_started()) {
            sessions_.VisitAll([=](auto k, std::shared_ptr<SessionWrapper>& sw) {
                if (sw->session_) {
                    sw->session_->async_send(msg);
                }
            });
        }
    }
}
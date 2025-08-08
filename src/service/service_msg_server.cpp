//
// Created by RGAA on 29/11/2024.
//

#include "service_msg_server.h"
#include "service.h"
#include "tc_common_new/log.h"
#include "render_manager.h"
#include "tc_common_new/url_helper.h"
#include "tc_common_new/file.h"
#include "tc_service_message.pb.h"
#include "service_context.h"

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
        server_ = std::make_shared<asio2::wss_server>();

        auto exe_dir = context_->GetAppExeFolderPath();
        auto pwd_file = std::format("{}/certs/password", exe_dir);
        auto pwd = tc::File::OpenForRead(pwd_file)->ReadAllAsString();
        server_->set_cert_file(
                "",
                std::format("{}/certs/server.crt", exe_dir),
                std::format("{}/certs/server.key", exe_dir),
                pwd);

        if (asio2::get_last_error()) {
            LOGE("load cert files failed: {}", asio2::last_error_msg());
        }
        else {
            LOGE("set cert files success.");
        }

        //  | asio::ssl::verify_fail_if_no_peer_cert
        server_->set_verify_mode(asio::ssl::verify_peer);

        auto fn_get_socket_fd = [](std::shared_ptr<asio2::wss_session> &sess_ptr) -> uint64_t {
            auto& s = sess_ptr->socket();
            return (uint64_t)s.native_handle();
        };

        server_->bind_accept([&](std::shared_ptr<asio2::wss_session>& session_ptr) {
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
       .bind_connect([=, this](std::shared_ptr<asio2::wss_session>& sess_ptr) {
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

        bool ret = server_->start("0.0.0.0", context_->GetListeningPort());
        LOGI("service start at: 0.0.0.0:{}/service/message, result: {}", context_->GetListeningPort(), ret);
    }

    void ServiceMsgServer::ParseMessage(const std::shared_ptr<SessionWrapper>& sw, std::string_view data) {
        tc::ServiceMessage msg;
        try {
            msg.ParseFromString(std::string(data.data(), data.size()));
            auto type = msg.type();
            if (type == ServiceMessageType::kSrvStartServer) {
                auto sub = msg.start_server();
                std::vector<std::string> args;
                for (int i = 0; i < sub.args_size(); i++) {
                    args.push_back(sub.args(i));
                }
                ProcessStartRender(sub.work_dir(), sub.app_path(), args);
            }
            else if (type == ServiceMessageType::kSrvStopServer) {
                ProcessStopRender();
            }
            else if (type == ServiceMessageType::kSrvRestartServer) {
                auto sub = msg.restart_server();
                std::vector<std::string> args;
                for (int i = 0; i < sub.args_size(); i++) {
                    args.push_back(sub.args(i));
                }
                ProcessRestartRender(sub.work_dir(), sub.app_path(), args);
            }
            else if (type == ServiceMessageType::kSrvHeartBeat) {
                auto sub = msg.heart_beat();
                sw->from_ = sub.from();
                ProcessHeartBeat(sub.index());
            }
            else if (type == ServiceMessageType::kSrvReqCtrlAltDelete) {
                auto sub = msg.req_ctrl_alt_delete();
                ProcessCtrlAltDelete();
                LOGI("client, id:{}, device id: {}, request control alt delete", sub.req_device_id(), sub.req_stream_id());
            }
        }
        catch(...) {
            LOGE("Parse message failed!");
        }
    }

    void ServiceMsgServer::ProcessStartRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args) {
        if (!render_manager_->StartDesktopRender(work_dir, app_path, args)) {
            LOGE("Start server failed!");
        }
    }

    void ServiceMsgServer::ProcessStopRender() {
        render_manager_->StopDesktopRender();
    }

    void ServiceMsgServer::ProcessRestartRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args) {
        render_manager_->ReStartDesktopRender(work_dir, app_path, args);
    }

    void ServiceMsgServer::ProcessHeartBeat(int64_t index) {
        auto is_render_alive = render_manager_->IsDesktopRenderAlive();
        ServiceMessage msg;
        msg.set_type(ServiceMessageType::kSrvHeartBeatResp);
        auto sub = msg.mutable_heart_beat_resp();
        sub->set_index(index);
        sub->set_render_status(is_render_alive ? RenderStatus::kWorking : RenderStatus::kStopped);
        PostBinaryMessage(msg.SerializeAsString());
    }

    void ServiceMsgServer::ProcessCtrlAltDelete() {
        service_->SimulateCtrlAltDelete();
    }

    void ServiceMsgServer::PostBinaryMessage(const std::string& msg) {
        if (server_ && server_->is_started()) {
            sessions_.VisitAll([=](auto k, std::shared_ptr<SessionWrapper>& sw) {
                if (sw->session_) {
                    sw->session_->async_send(msg);
                }
            });
        }
    }
}
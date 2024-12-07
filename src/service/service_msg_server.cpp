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
        http_server_ = std::make_shared<asio2::http_server>();
        http_server_->support_websocket(true);
        http_server_->bind_disconnect([=, this](std::shared_ptr<asio2::http_session>& sess_ptr) {
            auto socket_fd = (uint64_t)sess_ptr->socket().native_handle();
            if (sessions_.HasKey(socket_fd)) {
                sessions_.Remove(socket_fd);
                LOGI("client close, session size: {}", sessions_.Size());
            }
        });

        auto fn_get_socket_fd = [](std::shared_ptr<asio2::http_session> &sess_ptr) -> uint64_t {
            auto& s = sess_ptr->socket();
            return (uint64_t)s.native_handle();
        };

        http_server_->bind(service_path_, websocket::listener<asio2::http_session>{}
            .on("message", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr, std::string_view data) {
                this->ParseMessage(data);
            })
            .on("open", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
                sess_ptr->keep_alive(true);
                auto query = sess_ptr->get_request().get_query();
                auto params = UrlHelper::ParseQueryString(std::string(query.data(), query.size()));
                std::string from;
                for (const auto& [k, v] : params) {
                    LOGI("query param, k: {}, v: {}", k, v);
                    if (k == "from") {
                        from = v;
                    }
                }
                LOGI("Service server {} open, query: {}", service_path_, query);
                bool only_audio = std::atoi(params["only_audio"].c_str()) == 1;
                sess_ptr->set_no_delay(true);
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                auto sess_wrapper = std::make_shared<SessionWrapper>();
                sess_wrapper->from_ = from;
                sess_wrapper->socket_fd_ = socket_fd;
                sess_wrapper->session_ = sess_ptr;
                sessions_.Insert(socket_fd, sess_wrapper);
            })
            .on("close", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                LOGI("client closed: {}", socket_fd);
                sessions_.Remove(socket_fd);
            })
            .on_ping([=, this](auto &sess_ptr) {

            })
            .on_pong([=, this](auto &sess_ptr) {

            })
        );
        bool ret = http_server_->start("0.0.0.0", 20375);
        LOGI("service start at: 0.0.0.0:20375/service/message, result: {}", ret);
    }

    void ServiceMsgServer::ParseMessage(std::string_view data) {
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
                ProcessRestartRender();
            }
            else if (type == ServiceMessageType::kHeartBeat) {
                auto sub = msg.heart_beat();
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

    void ServiceMsgServer::ProcessRestartRender() {
        render_manager_->ReStart();
    }

    void ServiceMsgServer::ProcessHeartBeat(int64_t index) {
        LOGI("HeartBeat: {}", index);
        auto is_render_alive = render_manager_->IsRenderAlive();
        ServiceMessage msg;
        msg.set_type(ServiceMessageType::kHeartBeatResp);
        auto sub = msg.mutable_heart_beat_resp();
        sub->set_index(index);
        sub->set_render_status(is_render_alive ? RenderStatus::kWorking : RenderStatus::kStopped);
        PostBinaryMessage(msg.SerializeAsString());
    }

    void ServiceMsgServer::PostBinaryMessage(const std::string& msg) {
        if (http_server_ && http_server_->is_started()) {
            sessions_.VisitAll([=](auto k, std::shared_ptr<SessionWrapper>& sw) {
                if (sw->session_) {
                    sw->session_->async_send(msg);
                }
            });
        }
    }
}
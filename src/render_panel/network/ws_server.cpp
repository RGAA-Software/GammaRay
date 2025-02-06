//
// Created by RGAA on 2024-03-30.
//

#include "ws_server.h"
#include "render_panel/gr_settings.h"
#include "tc_common_new/log.h"
#include "tc_message.pb.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/gr_application.h"
#include "http_handler.h"
#include "apis.h"

namespace tc
{

    static std::string kUrlPanel = "/panel";

    struct aop_log {
        bool before(http::web_request &req, http::web_response &rep) {
            asio2::ignore_unused(rep);
            return true;
        }

        bool after(std::shared_ptr<asio2::http_session> &session_ptr, http::web_request &req, http::web_response &rep) {
                    ASIO2_ASSERT(asio2::get_current_caller<std::shared_ptr<asio2::http_session>>().get() == session_ptr.get());
            asio2::ignore_unused(session_ptr, req, rep);
            return true;
        }
    };

    std::shared_ptr<WSServer> WSServer::Make(const std::shared_ptr<GrApplication>& app) {
        return std::make_shared<WSServer>(app);
    }

    WSServer::WSServer(const std::shared_ptr<GrApplication>& app) {
        app_ = app;
        context_ = app_->GetContext();
        http_handler_ = std::make_shared<HttpHandler>(app_);
    }

    void WSServer::Start() {
        http_server_ = std::make_shared<asio2::http_server>();
        http_server_->bind_disconnect([=, this](std::shared_ptr<asio2::http_session>& sess_ptr) {
            auto socket_fd = (uint64_t)sess_ptr->socket().native_handle();
            if (sessions_.HasKey(socket_fd)) {
                sessions_.Remove(socket_fd);
                //NotifyMediaClientDisConnected();
                LOGI("client disconnected: {}", socket_fd);
                LOGI("App server media close, media router size: {}", sessions_.Size());
            }
            //this->NotifyPeerDisconnected();
        });

        http_server_->support_websocket(true);
        ws_data_ = std::make_shared<WsData>(WsData{
            .vars_ = {
                {"app",  this->app_},
            }
        });

        // response a "Pong" for checking server state
        AddHttpGetRouter(kPathPing, [=, this](const auto& path, auto& req, auto& rep) {
            http_handler_->HandlePing(req, rep);
        });

        // response the information that equals to the QR Code
        AddHttpGetRouter(kPathSimpleInfo, [=, this](const auto& path, auto& req, auto& rep) {
            http_handler_->HandleSimpleInfo(req, rep);
        });

        // response all apps that we found in system and added by user
        AddHttpGetRouter(kPathGames, [=, this](const auto& path, auto& req, auto& rep) {
            http_handler_->HandleGames(req, rep);
        });

        // start game
        AddHttpPostRouter(kPathGameStart, [=, this](const auto& path, auto& req, auto& rep) {
            http_handler_->HandleGameStart(req, rep);
        });

        // stop game
        AddHttpPostRouter(kPathGameStop, [=, this](const auto& path, auto& req, auto& rep) {
            http_handler_->HandleGameStop(req, rep);
        });

        // running games
        AddHttpGetRouter(kPathRunningGames, [=, this](const auto& path, auto& req, auto& rep) {
            http_handler_->HandleRunningGames(req, rep);
        });

        // stop the GammaRayRender.exe
        AddHttpGetRouter(kPathStopServer, [=, this](const auto& path, auto& req, auto& rep) {
            http_handler_->HandleStopServer(req, rep);
        });

        // all running processes in th PC, equals the process list in TaskManager
        AddHttpGetRouter(kPathAllRunningProcesses, [=, this](const auto& path, auto& req, auto& rep) {
            http_handler_->HandleAllRunningProcesses(req, rep);
        });

        // kill a process by pid
        AddHttpPostRouter(kPathKillProcess, [=, this](const auto& path, auto& req, auto& rep) {
            http_handler_->HandleKillProcess(req, rep);
        });

        // res
        AddHttpGetRouter("/res/*", [=, this](const auto& path, auto& req, auto& rep) {
            http_handler_->HandleResourcesFile(req, rep);
        });

        // cache
        AddHttpGetRouter("/steam/cache/*", [=, this](const auto& path, auto& req, auto& rep) {
            http_handler_->HandleSteamCacheFile(req, rep);
        });

        AddWebsocketRouter<std::shared_ptr<asio2::http_server>>(kUrlPanel, http_server_);

        bool ret = http_server_->start("0.0.0.0", GrSettings::Instance()->ws_server_port_);
        LOGI("App server start result: {}", ret);
    }

    void WSServer::Exit() {

    }

    WSServer::~WSServer() {
        if (http_server_) {
            http_server_->stop_all_timers();
            http_server_->stop();
        }
    }

    template<typename Server>
    void WSServer::AddWebsocketRouter(const std::string &path, const Server &s) {
        auto fn_get_socket_fd = [](std::shared_ptr<asio2::http_session> &sess_ptr) -> uint64_t {
            auto& s = sess_ptr->socket();
            return (uint64_t)s.native_handle();
        };
        s->bind(path, websocket::listener<asio2::http_session>{}
            .on("message", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr, std::string_view data) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                if (path == kUrlPanel) {
                    this->ParseBinaryMessage(socket_fd, data);
                }
            })
            .on("open", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
                LOGI("App server {} open", path);
                sess_ptr->ws_stream().binary(true);
                sess_ptr->set_no_delay(true);
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                if (path == kUrlPanel) {
                    auto ws_sess = std::make_shared<WSSession>();
                    ws_sess->socket_fd_ = socket_fd;
                    ws_sess->session_ = sess_ptr;
                    this->sessions_.Insert(socket_fd, ws_sess);
                    LOGI("client connect : {}", socket_fd);

                    //this->NotifyPeerConnected();
                }
            })
            .on("close", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                if (path == kUrlPanel) {
                    if (sessions_.HasKey(socket_fd)) {
                        sessions_.Remove(socket_fd);
                    }
                    //this->NotifyPeerDisconnected();
                }
            })
            .on_ping([=, this](auto &sess_ptr) {

            })
            .on_pong([=, this](auto &sess_ptr) {

            })
        );
    }

    void WSServer::AddHttpGetRouter(const std::string &path,
        std::function<void(const std::string& path, http::web_request &req, http::web_response &rep)>&& cbk) {
        http_server_->bind<http::verb::get>(path, [=, this](http::web_request &req, http::web_response &rep) {
            cbk(path, req, rep);
        }, aop_log{});
    }

    void WSServer::AddHttpPostRouter(const std::string& path,
        std::function<void(const std::string& path, http::web_request &req, http::web_response &rep)>&& cbk) {
        http_server_->bind<http::verb::post>(path, [=, this](http::web_request &req, http::web_response &rep) {
            cbk(path, req, rep);
        }, aop_log{});
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
            auto statistics = std::make_shared<CaptureStatistics>();
            statistics->CopyFrom(proto_msg->capture_statistics());
            context_->SendAppMessage(MsgCaptureStatistics{
                .msg_ = proto_msg,
                .statistics_ = statistics,
            });

        } else if (proto_msg->type() == tc::kServerAudioSpectrum) {
            //auto spectrum = proto_msg->server_audio_spectrum();
            auto spectrum = std::make_shared<ServerAudioSpectrum>();
            spectrum->CopyFrom(proto_msg->server_audio_spectrum());
            context_->SendAppMessage(MsgServerAudioSpectrum {
                .msg_ = proto_msg,
                .spectrum_ = spectrum,
            });

        }  else if (proto_msg->type() == tc::kRestartServer) {
            context_->SendAppMessage(AppMsgRestartServer {});
        }
    }

}
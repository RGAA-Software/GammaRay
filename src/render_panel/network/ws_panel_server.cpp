//
// Created by RGAA on 2024-03-30.
//

#include "ws_panel_server.h"
#include "apis.h"
#include "http_handler.h"
#include "render_panel/gr_settings.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_render_panel_message.pb.h"
#include "tc_client_panel_message.pb.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/gr_application.h"
#include "render_panel/transfer/file_transfer.h"
#include "render_panel/database/gr_database.h"
#include "render_panel/database/visit_record.h"
#include "render_panel/database/visit_record_operator.h"
#include "render_panel/database/file_transfer_record.h"
#include "render_panel/database/file_transfer_record_operator.h"
#include "tc_common_new/url_helper.h"
#include <QApplication>

namespace tc
{

    static std::string kUrlPanel = "/panel";
    static std::string kUrlPanelRenderer = "/panel/renderer";
    static std::string kUrlFileTransfer = "/file/transfer";

    struct aop_log {
        bool before(http::web_request &req, http::web_response &rep) {
            asio2::ignore_unused(rep);
            return true;
        }

        bool after(std::shared_ptr<asio2::https_session> &session_ptr, http::web_request &req, http::web_response &rep) {
                    ASIO2_ASSERT(asio2::get_current_caller<std::shared_ptr<asio2::https_session>>().get() == session_ptr.get());
            asio2::ignore_unused(session_ptr, req, rep);
            return true;
        }
    };

    std::shared_ptr<WsPanelServer> WsPanelServer::Make(const std::shared_ptr<GrApplication>& app) {
        return std::make_shared<WsPanelServer>(app);
    }

    WsPanelServer::WsPanelServer(const std::shared_ptr<GrApplication>& app) {
        app_ = app;
        context_ = app_->GetContext();
        http_handler_ = std::make_shared<HttpHandler>(app_);
        settings_ = GrSettings::Instance();
        visit_record_op_ = context_->GetDatabase()->GetVisitRecordOp();
        ft_record_op_ = context_->GetDatabase()->GetFileTransferRecordOp();
    }

    void WsPanelServer::Start() {
        server_ = std::make_shared<asio2::https_server>();
        server_->bind_disconnect([=, this](std::shared_ptr<asio2::https_session>& sess_ptr) {
            auto socket_fd = (uint64_t)sess_ptr->socket().native_handle();
            if (panel_sessions_.HasKey(socket_fd)) {
                panel_sessions_.Remove(socket_fd);
                LOGI("Panel;client disconnected: {}", socket_fd);
                LOGI("Panel;App server media close, media router size: {}", panel_sessions_.Size());
            }
            if (renderer_sessions_.HasKey(socket_fd)) {
                renderer_sessions_.Remove(socket_fd);
                LOGI("Renderer;client disconnected: {}", socket_fd);
                LOGI("Renderer;App server media close, media router size: {}", panel_sessions_.Size());
            }
        });

        server_->support_websocket(true);
        ws_data_ = std::make_shared<WsData>(WsData{
            .vars_ = {
                {"app",  this->app_},
            }
        });

        auto exe_dir = qApp->applicationDirPath().toStdString();
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

        // server_->set_dh_file(std::format("{}/certs/dh1024.pem", exe_dir));
        // if (asio2::get_last_error()) {
        //     LOGE("load dh files failed: ", asio2::last_error_msg());
        // }

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

        // panel
        AddWebsocketRouter(kUrlPanel);
        // panel/renderer
        AddWebsocketRouter(kUrlPanelRenderer);
        // file transfer
        AddWebsocketRouter(kUrlFileTransfer);

        server_->start_timer("id.sync.info", 1000, [=, this]() {
            this->RpSyncPanelInfo();
        });

        bool ret = server_->start("0.0.0.0", settings_->panel_srv_port_);
        LOGI("App server start result: {}, port: {}", ret, settings_->panel_srv_port_);
    }

    void WsPanelServer::Exit() {

    }

    WsPanelServer::~WsPanelServer() {
        if (server_) {
            server_->stop_all_timers();
            server_->stop();
        }
    }

    void WsPanelServer::AddWebsocketRouter(const std::string &path) {
        auto fn_get_socket_fd = [](std::shared_ptr<asio2::https_session> &sess_ptr) -> uint64_t {
            auto& s = sess_ptr->socket();
            return (uint64_t)s.native_handle();
        };
        server_->bind(path, websocket::listener<asio2::https_session>{}
            .on("message", [=, this](std::shared_ptr<asio2::https_session> &sess_ptr, std::string_view data) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                if (path == kUrlPanel) {
                    this->ParsePanelMessage(socket_fd, data);
                }
                else if (path == kUrlPanelRenderer) {
                    this->ParseRendererMessage(socket_fd, data);
                }
                else if (path == kUrlFileTransfer) {
                    this->ParseFtBinaryMessage(socket_fd, data);
                }
            })
            .on("open", [=, this](std::shared_ptr<asio2::https_session> &sess_ptr) {
                LOGI("App server {} open", path);
                sess_ptr->ws_stream().binary(true);
                sess_ptr->set_no_delay(true);
                auto socket_fd = fn_get_socket_fd(sess_ptr);

                auto query = sess_ptr->get_request().get_query();
                auto params = UrlHelper::ParseQueryString(std::string(query.data(), query.size()));

                if (path == kUrlPanel) {
                    for (const auto& [k, v] : params) {
                        LOGI("query param, k: {}, v: {}", k, v);
                    }
                    LOGI("App server {} open, query: {}", path, query);
                    std::string stream_id;
                    if (params.contains("stream_id")) {
                        stream_id = params["stream_id"];
                    }

                    auto ws_sess = std::make_shared<WSSession>();
                    ws_sess->socket_fd_ = socket_fd;
                    ws_sess->session_ = sess_ptr;
                    ws_sess->stream_id_ = stream_id;
                    this->panel_sessions_.Insert(socket_fd, ws_sess);
                    LOGI("Panel;client connect : {}", socket_fd);
                }
                else if (path == kUrlPanelRenderer) {
                    auto ws_sess = std::make_shared<WSSession>();
                    ws_sess->socket_fd_ = socket_fd;
                    ws_sess->session_ = sess_ptr;
                    this->renderer_sessions_.Insert(socket_fd, ws_sess);
                    LOGI("Renderer;client connect : {}", socket_fd);

                    sess_ptr->post_queued_event([=, this]() {
                        this->RpSyncPanelInfo();
                    });
                }
                else if (path == kUrlFileTransfer) {
                    auto ft_sess = std::make_shared<FtSession>();
                    ft_sess->socket_fd_ = socket_fd;
                    ft_sess->session_ = sess_ptr;
                    ft_sess->ch_ = std::make_shared<FileTransferChannel>(context_, sess_ptr);
                    this->ft_sessions_.Insert(socket_fd, ft_sess);
                    ft_sess->ch_->OnConnected();
                }
            })
            .on("close", [=, this](std::shared_ptr<asio2::https_session> &sess_ptr) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                if (path == kUrlPanel) {
                    if (panel_sessions_.HasKey(socket_fd)) {
                        panel_sessions_.Remove(socket_fd);
                    }
                }
                else if (path == kUrlPanelRenderer) {
                    if (renderer_sessions_.HasKey(socket_fd)) {
                        renderer_sessions_.Remove(socket_fd);
                    }
                }
                else if (path == kUrlFileTransfer) {
                    if (ft_sessions_.HasKey(socket_fd)) {
                        auto ft_session = ft_sessions_.Get(socket_fd);
                        ft_session->ch_->OnDisConnected();
                        ft_sessions_.Remove(socket_fd);
                    }
                }
            })
            .on_ping([=, this](auto &sess_ptr) {

            })
            .on_pong([=, this](auto &sess_ptr) {

            })
        );
    }

    bool WsPanelServer::IsAlive() {
        return server_ && server_->is_started();
    }

    void WsPanelServer::AddHttpGetRouter(const std::string &path,
        std::function<void(const std::string& path, http::web_request &req, http::web_response &rep)>&& cbk) {
        server_->bind<http::verb::get>(path, [=, this](http::web_request &req, http::web_response &rep) {
            cbk(path, req, rep);
        }, aop_log{});
    }

    void WsPanelServer::AddHttpPostRouter(const std::string& path,
        std::function<void(const std::string& path, http::web_request &req, http::web_response &rep)>&& cbk) {
        server_->bind<http::verb::post>(path, [=, this](http::web_request &req, http::web_response &rep) {
            cbk(path, req, rep);
        }, aop_log{});
    }

    void WsPanelServer::PostPanelMessage(const std::string& msg, bool only_inner) {
        panel_sessions_.VisitAll([=, this](uint64_t fd, std::shared_ptr<WSSession>& sess) {
            if (only_inner && sess->session_type_ != tccp::CpSessionType::kInnerServer) {
                return;
            }
            if (sess->session_) {
                sess->session_->async_send(msg);
            }
        });
    }

    bool WsPanelServer::ParsePanelMessage(uint64_t socket_fd, std::string_view msg) {
        auto proto_msg = std::make_shared<tccp::CpMessage>();
        if (!proto_msg->ParseFromArray(msg.data(), msg.size())) {
            return false;
        }
        if (proto_msg->type() == tccp::CpMessageType::kCpHello) {
            auto hello = proto_msg->hello();
            panel_sessions_.VisitAll([=](uint64_t k, std::shared_ptr<WSSession>& v) {
                if (v->socket_fd_ == socket_fd) {
                    v->session_type_ = hello.type();
                    LOGI("Update session type: {} for socket: {}", v->session_type_, socket_fd);
                }
            });

            context_->SendAppMessage(MsgClientConnectedPanel {
                .stream_id_ = proto_msg->stream_id(),
                .sess_type_ = hello.type(),
            });
        }
        else if (proto_msg->type() == tccp::CpMessageType::kCpHeartBeat) {
            auto hb = proto_msg->heartbeat();
            LOGI("HB: stream id: {} remote desktop: {} os: {}", proto_msg->stream_id(), hb.remote_device_desktop_name(), hb.remote_os_name());
            if (proto_msg->stream_id().empty() || hb.remote_device_desktop_name().empty() || hb.remote_os_name().empty()) {
                return false;
            }

            context_->SendAppMessage(MsgRemotePeerInfo {
                .stream_id_ = proto_msg->stream_id(),
                .desktop_name_ = hb.remote_device_desktop_name(),
                .os_version_ = hb.remote_os_name(),
            });
        }
        return true;
    }

    void WsPanelServer::RpSyncPanelInfo() {
        tcrp::RpMessage m;
        m.set_type(tcrp::RpMessageType::kSyncPanelInfo);
        auto sub = m.mutable_sync_panel_info();
        sub->set_device_id(settings_->device_id_);
        sub->set_device_random_pwd(settings_->device_random_pwd_);
        sub->set_device_safety_pwd(settings_->device_safety_pwd_);
        sub->set_relay_host(settings_->relay_server_host_);
        sub->set_relay_port(settings_->relay_server_port_);
        PostRendererMessage(m.SerializeAsString());
    }

    void WsPanelServer::ParseFtBinaryMessage(uint64_t socket_fd, std::string_view msg) {
        if (ft_sessions_.HasKey(socket_fd)) {
            auto sess = ft_sessions_.Get(socket_fd);
            sess->ch_->ParseBinaryMessage(msg);
        }
    }

    // to /panel/renderer socket
    void WsPanelServer::PostRendererMessage(const std::string& msg) {
        renderer_sessions_.VisitAll([=, this](uint64_t fd, std::shared_ptr<WSSession>& sess) {
            if (sess->session_) {
                sess->session_->async_send(msg);
            }
        });
    }

    // parse /panel/renderer socket
    void WsPanelServer::ParseRendererMessage(uint64_t socket_fd, std::string_view msg) {
        auto proto_msg = std::make_shared<tcrp::RpMessage>();
        if (!proto_msg->ParseFromArray(msg.data(), msg.size())) {
            LOGE("Parse binary message failed.");
            return;
        }
        if (proto_msg->type() == tcrp::kRpCaptureStatistics) {
            auto statistics = std::make_shared<tcrp::RpCaptureStatistics>();
            statistics->CopyFrom(proto_msg->capture_statistics());
            context_->SendAppMessage(MsgCaptureStatistics{
                .msg_ = proto_msg,
                .statistics_ = statistics,
            });
        }
        else if (proto_msg->type() == tcrp::kRpServerAudioSpectrum) {
            //auto spectrum = proto_msg->renderer_audio_spectrum();
            auto spectrum = std::make_shared<tcrp::RpServerAudioSpectrum>();
            spectrum->CopyFrom(proto_msg->renderer_audio_spectrum());
            context_->SendAppMessage(MsgServerAudioSpectrum {
                    .msg_ = proto_msg,
                    .spectrum_ = spectrum,
            });
        }
        else if (proto_msg->type() == tcrp::kRpRestartServer) {
            context_->SendAppMessage(AppMsgRestartServer {});
        }
        else if (proto_msg->type() == tcrp::kRpPluginsInfo) {
            auto plugins_info = std::make_shared<tcrp::RpPluginsInfo>();
            plugins_info->CopyFrom(proto_msg->plugins_info());
            context_->SendAppMessage(MsgPluginsInfo {
                .plugins_info_ = plugins_info,
            });
        }
        else if (proto_msg->type() == tcrp::kRpClientConnected) {
            context_->PostDBTask([=, this]() {
                auto ips = context_->GetIps();
                std::string ip_address;
                if (!ips.empty()) {
                    ip_address = ips[0].ip_addr_;
                }
                auto sub = proto_msg->client_connected();
                visit_record_op_->InsertVisitRecord(std::make_shared<VisitRecord>(VisitRecord {
                    .the_conn_id_ = sub.the_conn_id(),
                    .conn_type_ = sub.conn_type(),
                    .begin_ = sub.begin_timestamp(),
                    .end_ = 0,
                    .duration_ = 0,
                    .visitor_device_ = sub.device_id(),
                    .target_device_ = settings_->device_id_.empty() ? ip_address : settings_->device_id_,
                }));
            });
        }
        else if (proto_msg->type() == tcrp::kRpClientDisConnected) {
            context_->PostDBTask([=, this]() {
                auto sub = proto_msg->client_disconnected();
                visit_record_op_->UpdateVisitRecord(sub.the_conn_id(), sub.end_timestamp(), sub.duration());
            });
        }
    }

}
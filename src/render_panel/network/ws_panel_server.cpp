//
// Created by RGAA on 2024-03-30.
//

#include "ws_panel_server.h"
#include <QApplication>
#include "apis.h"
#include "http_handler.h"
#include "tc_message.pb.h"
#include "render_panel/gr_settings.h"
#include "json/json.hpp"
#include "tc_common_new/http_client.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/file.h"
#include "tc_render_panel_message.pb.h"
#include "tc_client_panel_message.pb.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_render_msg_processor.h"
#include "render_panel/transfer/file_transfer.h"
#include "render_panel/database/gr_database.h"
#include "render_panel/database/visit_record.h"
#include "render_panel/database/visit_record_operator.h"
#include "render_panel/database/file_transfer_record.h"
#include "render_panel/database/file_transfer_record_operator.h"
#include "tc_message_new/rp_proto_converter.h"
#include "tc_common_new/url_helper.h"
#include "tc_common_new/message_notifier.h"
#include "tc_qt_widget/translator/tc_translator.h"
#include "render_panel/companion/panel_companion.h"
#include "render_panel/gr_statistics.h"
#include "render_panel/devices/gr_device_manager.h"
#include "skin/interface/skin_interface.h"

namespace tc
{

    static std::string kUrlPanel = "/panel";
    static std::string kUrlPanelRenderer = "/panel/renderer";
    static std::string kUrlFileTransfer = "/file/transfer";
    static std::string kUrlSysInfo = "/sys/info";

    // report visit info to cms
    static const std::string kUrlVisitRecord  = "/api/v1/record/upload_visit_info";

    static const std::string kUrlUpdateVisitRecord = "/api/v1/record/update_visit_info";

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

    std::shared_ptr<WsPanelServer> WsPanelServer::Make(const std::shared_ptr<GrApplication>& app) {
        return std::make_shared<WsPanelServer>(app);
    }

    WsPanelServer::WsPanelServer(const std::shared_ptr<GrApplication>& app) {
        app_ = app;
        stat_ = GrStatistics::Instance();
        context_ = app_->GetContext();
        http_handler_ = std::make_shared<HttpHandler>(app_);
        settings_ = GrSettings::Instance();
        visit_record_op_ = context_->GetDatabase()->GetVisitRecordOp();
        ft_record_op_ = context_->GetDatabase()->GetFileTransferRecordOp();

        msg_listener_ = context_->ObtainMessageListener();
        msg_listener_->Listen<MsgSecurityPasswordUpdated>([=, this](const MsgSecurityPasswordUpdated& msg) {
            context_->PostTask([=, this]() {
                this->RpSyncPanelInfo();
            });
        });

        msg_listener_->Listen<MsgGrTimer1S>([=, this](const MsgGrTimer1S& msg) {
            context_->PostTask([=, this]() {
                this->RpSyncPanelInfo();
            });
        });

        msg_listener_->Listen<MsgGrTimer5S>([=, this](const MsgGrTimer5S& msg) {
            context_->PostTask([=, this]() {
                if (panel_sessions_.Size() > 0) {
                    app_->GetDeviceManager()->UpdateUsedTime(5000);
                }
            });
        });

        msg_listener_->Listen<MsgHWInfo>([=, this](const MsgHWInfo& msg) {
            if (msg.sys_info_->networks_.empty()) {
                return;
            }
            const auto& def_ethernet = msg.sys_info_->networks_[0];
            this->max_transmit_speed_ = def_ethernet.max_transmit_speed_;
            this->max_receive_speed_ = def_ethernet.max_receive_speed_;
        });
    }

    void WsPanelServer::Start() {
        server_ = std::make_shared<asio2::http_server>();
        server_->bind_disconnect([=, this](std::shared_ptr<asio2::http_session>& sess_ptr) {
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

        //auto exe_dir = qApp->applicationDirPath().toStdString();
        //auto pwd_file = std::format("{}/certs/password", exe_dir);
        //auto pwd = tc::File::OpenForRead(pwd_file)->ReadAllAsString();
        //server_->set_cert_file(
        //    "",
        //    std::format("{}/certs/server.crt", exe_dir),
        //    std::format("{}/certs/server.key", exe_dir),
        //    pwd
        //);

        //if (asio2::get_last_error()) {
        //    LOGE("load cert files failed: {}", asio2::last_error_msg());
        //}
        //else {
        //    LOGE("set cert files success.");
        //}

        ////  | asio::ssl::verify_fail_if_no_peer_cert
        //server_->set_verify_mode(asio::ssl::verify_peer);

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

        // default
        server_->bind<http::verb::get, http::verb::post>("/", [](http::web_request& req, http::web_response& rep) {
            asio2::ignore_unused(req, rep);
            rep.fill_file("/web/index.html");
        }, aop_log{});

        // If no method is specified, GET and POST are both enabled by default.
        server_->bind("*", [](http::web_request& req, http::web_response& rep) {
            rep.fill_file("/web" + http::url_decode(req.target()));
            rep.chunked(true);
        }, aop_log{});

        // panel
        AddWebsocketRouter(kUrlPanel);
        // panel/renderer
        AddWebsocketRouter(kUrlPanelRenderer);
        // file transfer
        AddWebsocketRouter(kUrlFileTransfer);
        // sys info
        AddWebsocketRouter(kUrlSysInfo);

        bool ret = server_->start("0.0.0.0", settings_->GetPanelServerPort());
        LOGI("App server start result: {}, port: {}", ret, settings_->GetPanelServerPort());
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
        auto fn_get_socket_fd = [](std::shared_ptr<asio2::http_session> &sess_ptr) -> uint64_t {
            auto& s = sess_ptr->socket();
            return (uint64_t)s.native_handle();
        };
        server_->bind(path, websocket::listener<asio2::http_session>{}
            .on("message", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr, std::string_view data) {
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
                else if (path == kUrlSysInfo) {
                    if (sys_info_sess_) {
                        this->ParseSysInfoMessage(socket_fd, data);
                    }
                }
            })
            .on("open", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
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
                else if (path == kUrlSysInfo) {
                    auto sess = std::make_shared<WSSession>();
                    sess->socket_fd_ = socket_fd;
                    sess->session_ = sess_ptr;
                    sys_info_sess_ = sess;
                }
            })
            .on("close", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
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
                else if (path == kUrlSysInfo) {
                    if (sys_info_sess_) {
                        sys_info_sess_.reset();
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
            //LOGI("HB: stream id: {} remote desktop: {} os: {}", proto_msg->stream_id(), hb.remote_device_desktop_name(), hb.remote_os_name());
            if (proto_msg->stream_id().empty() || hb.remote_device_desktop_name().empty() || hb.remote_os_name().empty()) {
                return false;
            }

            context_->SendAppMessage(MsgRemotePeerInfo {
                .stream_id_ = proto_msg->stream_id(),
                .desktop_name_ = hb.remote_device_desktop_name(),
                .os_version_ = hb.remote_os_name(),
            });
        }
        else if (proto_msg->type() == tccp::kCpFileTransferBegin) {
            context_->PostDBTask([=, this]() {
                auto sub = proto_msg->ft_transfer_beg();
                auto ips = context_->GetIps();
                std::string ip_address;
                if (!ips.empty()) {
                    ip_address = ips[0].ip_addr_;
                }

                auto record = std::make_shared<FileTransferRecord>(FileTransferRecord{
                    .the_file_id_ = sub.the_file_id(),
                    .begin_ = sub.begin_timestamp(),
                    .end_ = 0,
                    .visitor_device_ = settings_->GetDeviceId().empty() ? ip_address : settings_->GetDeviceId(),
                    .target_device_ = sub.remote_device_id(),
                    .direction_ = sub.direction(),
                    .file_detail_ = sub.file_detail(),
                });

                ft_record_op_->InsertFileTransferRecord(record);

                NotifyInsertFileTransferRecordToCms(record);
            });
        }
        else if (proto_msg->type() == tccp::kCpFileTransferEnd) {
            context_->PostDBTask([=, this]() {
                auto sub = proto_msg->ft_transfer_end();
                ft_record_op_->UpdateVisitRecord(sub.the_file_id(), sub.end_timestamp(), sub.success());

                auto record = std::make_shared<FileTransferRecord>(FileTransferRecord{
                    .the_file_id_ = sub.the_file_id(),
                    .end_ = sub.end_timestamp(),
                    .success_ = sub.success()
                });

                NotifyUpdateFileTransferRecordToCms(record);
            });
        }
        return true;
    }

    void WsPanelServer::RpSyncPanelInfo() {
        tcrp::RpMessage m;
        m.set_type(tcrp::RpMessageType::kSyncPanelInfo);
        auto sub = m.mutable_sync_panel_info();
        sub->set_device_id(settings_->GetDeviceId());
        sub->set_device_random_pwd(settings_->GetDeviceRandomPwd());
        sub->set_device_safety_pwd(settings_->GetDeviceSecurityPwd());
        sub->set_relay_host(settings_->GetRelayServerHost());
        sub->set_relay_port(std::to_string(settings_->GetRelayServerPort()));
        sub->set_can_be_operated(settings_->IsBeingOperatedEnabled());
        sub->set_relay_enabled(settings_->IsRelayEnabled());
        sub->set_language((int)tcTrMgr()->GetSelectedLanguage());
        sub->set_file_transfer_enabled(settings_->IsFileTransferEnabled());
        sub->set_audio_enabled(settings_->IsCaptureAudioEnabled());
        sub->set_appkey(grApp->GetAppkey());
        sub->set_max_transmit_speed(this->max_transmit_speed_);
        sub->set_max_receive_speed(this->max_receive_speed_);
        if (auto pc = grApp->GetCompanion(); pc) {
            sub->set_role(static_cast<int>(pc->GetAuth()->role_));
        }
        else {
            sub->set_role(1);
        }
        PostRendererMessage(tc::RpProtoAsData(&m));
    }

    void WsPanelServer::ParseFtBinaryMessage(uint64_t socket_fd, std::string_view msg) {
        if (ft_sessions_.HasKey(socket_fd)) {
            auto sess = ft_sessions_.Get(socket_fd);
            sess->ch_->ParseBinaryMessage(msg);
        }
    }

    // to /panel/renderer socket
    void WsPanelServer::PostRendererMessage(std::shared_ptr<Data> msg) {
        renderer_sessions_.VisitAll([=](uint64_t fd, std::shared_ptr<WSSession>& sess) {
            if (sess->session_) {
                sess->session_->async_send(msg->AsString());
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
                if (sub.visitor_device_id().empty()) {
                    return;
                }
                auto record = std::make_shared<VisitRecord>(VisitRecord{
                    .conn_id_ = sub.conn_id(),
                    .stream_id_ = sub.stream_id(),
                    .conn_type_ = sub.conn_type(),
                    .begin_ = sub.begin_timestamp(),
                    .end_ = 0,
                    .duration_ = 0,
                    .visitor_device_ = sub.visitor_device_id(),
                    .target_device_ = settings_->GetDeviceId().empty() ? ip_address : settings_->GetDeviceId(),
                });
                visit_record_op_->InsertVisitRecord(record);
                // notify cms
                NotifyInsertVisitRecordToCms(record);
            });
        }
        else if (proto_msg->type() == tcrp::kRpClientDisConnected) {
            context_->PostDBTask([=, this]() {
                auto sub = proto_msg->client_disconnected();
                visit_record_op_->UpdateVisitRecord(sub.conn_id(), sub.end_timestamp(), sub.duration());

                auto record = std::make_shared<VisitRecord>(VisitRecord{
                    .conn_id_ = sub.conn_id(),
                    .end_ = sub.end_timestamp(),
                    .duration_ = sub.duration(),
                });
                NotifyUpdateVisitRecordToCms(record);
            });
            context_->SendAppMessage(MsgOneClientDisconnect{});
        }
        else if (proto_msg->type() == tcrp::kRpFileTransferBegin) {
            context_->PostDBTask([=, this]() {
                auto sub = proto_msg->ft_begin();
                auto ips = context_->GetIps();
                std::string ip_address;
                if (!ips.empty()) {
                    ip_address = ips[0].ip_addr_;
                }

                ft_record_op_->InsertFileTransferRecord(std::make_shared<FileTransferRecord>(FileTransferRecord {
                    .the_file_id_ = sub.the_file_id(),
                    .begin_ = sub.begin_timestamp(),
                    .end_ = 0,
                    .visitor_device_ = sub.visitor_device_id(),
                    .target_device_ = settings_->GetDeviceId().empty() ? ip_address : settings_->GetDeviceId(),
                    .direction_ = sub.direction(),
                    .file_detail_ = sub.file_detail(),
                }));
            });
        }
        else if (proto_msg->type() == tcrp::kRpFileTransferEnd) {
            context_->PostDBTask([=, this]() {
                auto sub = proto_msg->ft_end();
                ft_record_op_->UpdateVisitRecord(sub.the_file_id(), sub.end_timestamp(), sub.success());
            });
        }
        else if (proto_msg->type() == tcrp::kRpRawRenderMessage) {
            auto sub = proto_msg->raw_render_msg();
            auto rd_proto_msg = std::make_shared<tc::Message>();
            if (!rd_proto_msg->ParseFromString(sub.msg())) {
                LOGE("kRpRawRenderMessage parse failed");
                return;
            }
            auto processor = app_->GetRenderMsgProcessor();
            processor->OnMessage(rd_proto_msg);
        }
        else if (proto_msg->type() == tcrp::kRpRelayAlive) {
            auto sub = proto_msg->relay_alive();
            stat_->UpdateRelayAlive(sub.device_id(), sub.timestamp());
        }
    }

    void WsPanelServer::ParseSysInfoMessage(uint64_t socket_fd, std::string_view msg) {
        auto companion = app_->GetCompanion();
        if (!companion) {
            return;
        }

        std::string m = std::string(msg.data(), msg.size());
        auto sys_info = companion->ParseHardwareInfo(m);
        if (!sys_info) {
            return;
        }

        context_->SendAppMessage(MsgHWInfo {
            .sys_info_ = sys_info,
        });

        // to render
        {
            tcrp::RpMessage rp_msg;
            rp_msg.set_type(tcrp::RpMessageType::kRpHardwareInfo);
            auto sub = rp_msg.mutable_hw_info();
            sub->set_json_msg(sys_info->raw_json_msg_);
            sub->set_current_cpu_freq(companion->GetCurrentCpuFrequency());
            PostRendererMessage(tc::RpProtoAsData(&rp_msg));
        }
    }

    void WsPanelServer::NotifyInsertVisitRecordToCms(const std::shared_ptr<VisitRecord> record) {
        if (!record) {
            return;
        }
        auto settings = GrSettings::Instance();
        std::string serv_host = settings->GetSpvrServerHost();
        auto client = HttpClient::MakeSSL(serv_host, settings->GetSpvrServerPort(), kUrlVisitRecord, 2000);
        auto appkey = grApp->GetAppkey();
        auto resp = client->Post({
            {"appkey", appkey}
            }, record->AsJson2(), "application/json");

        if (resp.status != 200 || resp.body.empty()) {
            LOGE("NotifyInsertVisitRecordToCms failed: {}", resp.status);
        }
    }

    void WsPanelServer::NotifyUpdateVisitRecordToCms(const std::shared_ptr<VisitRecord> record) {
        if (!record) {
            return;
        }
        auto settings = GrSettings::Instance();
        std::string serv_host = settings->GetSpvrServerHost();
        auto client = HttpClient::MakeSSL(serv_host, settings->GetSpvrServerPort(), kUrlUpdateVisitRecord, 2000);
        auto appkey = grApp->GetAppkey();
        auto resp = client->Post({
            {"appkey", appkey}
            }, record->AsUpdateJson(), "application/json");

        if (resp.status != 200 || resp.body.empty()) {
            LOGE("NotifyUpdateVisitRecordToCms failed: {}", resp.status);
        }
    }

    void WsPanelServer::NotifyInsertFileTransferRecordToCms(const std::shared_ptr<FileTransferRecord> record) {
        if (!record) {
            return;
        }
        auto settings = GrSettings::Instance();
        std::string serv_host = settings->GetSpvrServerHost();
        auto client = HttpClient::MakeSSL(serv_host, settings->GetSpvrServerPort(), FileTransferRecord::kUrlInsertFileTransferRecord, 2000);
        auto appkey = grApp->GetAppkey();
        auto resp = client->Post({
            {"appkey", appkey}
            }, record->AsJson2(), "application/json");

        if (resp.status != 200 || resp.body.empty()) {
            LOGE("NotifyInsertFileTransferRecordToCms failed: {}", resp.status);
        }
    }

    void WsPanelServer::NotifyUpdateFileTransferRecordToCms(const std::shared_ptr<FileTransferRecord> record) {
        if (!record) {
            return;
        }
        auto settings = GrSettings::Instance();
        std::string serv_host = settings->GetSpvrServerHost();
        auto client = HttpClient::MakeSSL(serv_host, settings->GetSpvrServerPort(), FileTransferRecord::kUrlUpdateFileTransferRecord, 2000);
        auto appkey = grApp->GetAppkey();
        auto resp = client->Post({
            {"appkey", appkey}
            }, record->AsUpdateJson(), "application/json");

        if (resp.status != 200 || resp.body.empty()) {
            LOGE("NotifyUpdateFileTransferRecordToCms failed: {}", resp.status);
        }
    }
}
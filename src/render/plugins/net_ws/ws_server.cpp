//
// Created by RGAA on 2024/3/1.
//

#include "ws_server.h"

#include <memory>
#include "tc_common_new/log.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/data.h"
#include "tc_common_new/file.h"
#include "render/network/ws_media_router.h"
#include "ws_stream_router.h"
#include "ws_filetransfer_router.h"
#include "plugin_interface/gr_plugin_events.h"
#include "ws_plugin.h"
#include "tc_common_new/url_helper.h"
#include "http_handler.h"
#include <QApplication>

static std::string kUrlMedia = "/media";
static std::string kUrlFileTransfer = "/file/transfer";
static std::string kApiPing = "/api/ping";
static std::string kApiVerifySecurityPassword = "/verify/security/password";
static std::string kApiGetRenderConfiguration = "/get/render/configuration";
static std::string kApiPanelStreamMessage = "/panel/stream/message";

namespace tc
{

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

    WsPluginServer::WsPluginServer(tc::WsPlugin* plugin, uint16_t listen_port){
        this->plugin_ = plugin;
        this->listen_port_ = listen_port;
        http_handler_ = std::make_shared<HttpHandler>(plugin_);
    }

    void WsPluginServer::Start() {
        server_ = std::make_shared<asio2::http_server>();
        server_->bind_disconnect([=, this](std::shared_ptr<asio2::http_session>& sess_ptr) {
            auto socket_fd = (uint64_t)sess_ptr->socket().native_handle();
            //LOGI("client disconnected: {}", socket_fd);
            if (stream_routers_.HasKey(socket_fd)) {
                if (auto opt_val = stream_routers_.Remove(socket_fd); opt_val.has_value()) {
                    const auto& val = opt_val.value();
                    NotifyMediaClientDisConnected(val->conn_id_, val->stream_id_, val->visitor_device_id_, val->created_timestamp_);
                    LOGI("client session removed: {}", val->visitor_device_id_);
                }
                LOGI("App server media close, media router size: {}", stream_routers_.Size());
            }
            else if (ft_routers_.HasKey(socket_fd)) {
                ft_routers_.Remove(socket_fd);
            }
        });

        server_->support_websocket(true);
        ws_data_ = std::make_shared<WsData>(WsData{
            .vars_ = {
                {"plugin",  this->plugin_},
            }
        });

        //auto exe_dir = qApp->applicationDirPath().toStdString();
        //auto pwd_file = std::format("{}/certs/password", exe_dir);
        //auto pwd = (File::OpenForRead(pwd_file))->ReadAllAsString();
        //server_->set_cert_file(
        //    "",
        //    std::format("{}/certs/server.crt", exe_dir),
        //    std::format("{}/certs/server.key", exe_dir),
        //    pwd);

        //if (asio2::get_last_error()) {
        //    LOGE("load cert files failed: {}", asio2::last_error_msg());
        //}
        //else {
        //    LOGE("set cert files success.");
        //}
        //server_->set_verify_mode(asio::ssl::verify_peer);

        // media websocket
        AddWebsocketRouter(kUrlMedia);
        AddWebsocketRouter(kUrlFileTransfer);

        // ping
        AddHttpRouter(kApiPing, [=, this](const std::string& path, http::web_request& req, http::web_response& rep) {
            http_handler_->HandlePing(req, rep);
        });

        // verify security pwd
        AddHttpRouter(kApiVerifySecurityPassword, [=, this](const std::string& path, http::web_request& req, http::web_response& rep) {
            http_handler_->HandleVerifySecurityPassword(req, rep);
        });

        // get render configuration
        AddHttpRouter(kApiGetRenderConfiguration, [=, this](const std::string& path, http::web_request& req, http::web_response& rep) {
            http_handler_->HandleGetRenderConfiguration(req, rep);
        });

        //
        AddHttpRouter(kApiPanelStreamMessage, [=, this](const std::string& path, http::web_request& req, http::web_response& rep) {
            http_handler_->HandlePanelStreamMessage(req, rep);
        });

        if (listen_port_ <= 0) {
            LOGE("Listen port invalid: {}", listen_port_);
        }
        bool ret = server_->start("0.0.0.0", std::to_string(listen_port_));
        LOGI("App server start result: {}, listen port: {}", ret, listen_port_);
    }

    void WsPluginServer::Exit() {

    }

    void WsPluginServer::PostNetMessage(std::shared_ptr<Data> msg) {
        stream_routers_.ApplyAll([=](const uint64_t& socket_fd, const std::shared_ptr<WsStreamRouter>& router) {
            router->PostBinaryMessage(msg);
        });
    }

    bool WsPluginServer::PostTargetStreamMessage(const std::string& stream_id, std::shared_ptr<Data> msg) {
        bool found_target_stream = false;
        stream_routers_.ApplyAll([=, &found_target_stream](const uint64_t& socket_fd, const std::shared_ptr<WsStreamRouter>& router) {
            if (stream_id == router->stream_id_ || stream_id.empty()) {
                router->PostBinaryMessage(msg);
                found_target_stream = true;
            }
        });
        return found_target_stream;
    }

    bool WsPluginServer::PostTargetFileTransferMessage(const std::string& stream_id, std::shared_ptr<Data> msg) {
        bool found_target_stream = false;
        ft_routers_.ApplyAll([=, &found_target_stream](const uint64_t& socket_fd, const std::shared_ptr<WsFileTransferRouter>& router) {
            if (stream_id == router->stream_id_ || stream_id.empty()) {
                router->PostBinaryMessage(msg);
                found_target_stream = true;
            }
        });
        return found_target_stream;
    }

    int WsPluginServer::GetConnectedClientsCount() {
        return (int)stream_routers_.Size();
    }

    bool WsPluginServer::IsOnlyAudioClients() {
        bool only_audio_client = true;
        stream_routers_.VisitAllCond([&](auto k, auto& v) -> bool {
            if (v->enable_video_) {
                only_audio_client = false;
                return true;
            }
            return false;
        });
        return only_audio_client;
    }

    bool WsPluginServer::IsWorking() {
        return server_ && server_->is_started();
    }

    void WsPluginServer::AddWebsocketRouter(const std::string &path) {
        auto fn_get_socket_fd = [](std::shared_ptr<asio2::http_session> &sess_ptr) -> uint64_t {
            auto& s = sess_ptr->socket();
            return (uint64_t)s.native_handle();
        };
        server_->bind(path, websocket::listener<asio2::http_session>{}
            .on("message", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr, std::string_view data) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                if (path == kUrlMedia) {
                    stream_routers_.VisitAll([=](auto k, std::shared_ptr<WsStreamRouter>& router) mutable {
                        if (socket_fd == k) {
                            router->OnMessage(sess_ptr, socket_fd, data);
                        }
                    });
                }
                else if (path == kUrlFileTransfer) {
                    ft_routers_.VisitAll([=](auto k, auto &router) mutable {
                        if (socket_fd == k) {
                            router->OnMessage(sess_ptr, socket_fd, data);
                        }
                    });
                }
            })
            .on("open", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
                auto query = sess_ptr->get_request().get_query();
                auto params = UrlHelper::ParseQueryString(std::string(query.data(), query.size()));
                for (const auto& [k, v] : params) {
                    LOGI("query param, k: {}, v: {}", k, v);
                }
                LOGI("App server {} open, query: {}", path, query);
                bool only_audio = std::atoi(params["only_audio"].c_str()) == 1;
                std::string server_device_id;
                std::string visitor_device_id;
                std::string stream_id;
                if (params.contains("remote_device_id")) {
                    server_device_id = params["remote_device_id"];
                }
                if (params.contains("visitor_device_id")) {
                    visitor_device_id = params["visitor_device_id"];
                }
                if (params.contains("stream_id")) {
                    stream_id = params["stream_id"];
                }

                // TEST //
                if (stream_id.empty()) {
                    LOGE("!!!MUST HAVE STREAM ID!!!");
                    sess_ptr->stop();
                    return;
                }
                // TEST //

                sess_ptr->set_no_delay(true);
                auto socket_fd = fn_get_socket_fd(sess_ptr);

                if (path == kUrlMedia) {
                    auto router = WsStreamRouter::Make(ws_data_, only_audio, visitor_device_id, stream_id);
                    stream_routers_.Insert(socket_fd, router);
                    NotifyMediaClientConnected(router->conn_id_, router->stream_id_, visitor_device_id);
                    router->OnOpen(sess_ptr);
                }
                else if (path == kUrlFileTransfer) {
                    auto router = WsFileTransferRouter::Make(ws_data_, only_audio, visitor_device_id, stream_id);
                    ft_routers_.Insert(socket_fd, router);
                    router->OnOpen(sess_ptr);
                }

            })
            .on("close", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                LOGI("client closed: {}", socket_fd);
                if (path == kUrlMedia) {
                    if (auto opt_val = stream_routers_.Remove(socket_fd); opt_val.has_value()) {
                        const auto& val = opt_val.value();
                        NotifyMediaClientDisConnected(val->conn_id_, val->stream_id_, val->visitor_device_id_, val->created_timestamp_);
                        LOGI("client session removed: {}", val->visitor_device_id_);
                    }
                }
                else if (path == kUrlFileTransfer) {
                    ft_routers_.Remove(socket_fd);
                }
            })
            .on_ping([=, this](auto &sess_ptr) {

            })
            .on_pong([=, this](auto &sess_ptr) {

            })
            .on("update", [](std::shared_ptr<asio2::http_session> &sess_ptr) {
                LOGI("update");
            })
        );
    }

    void WsPluginServer::AddHttpRouter(const std::string &path,
       std::function<void(const std::string& path, http::web_request& req, http::web_response& rep)>&& callback) {
        // bind it
        server_->bind<http::verb::get, http::verb::post>(path, [=, this](http::web_request &req, http::web_response &rep) {
            callback(path, req, rep);
        }, aop_log{}); //, http::enable_cache
    }

    void WsPluginServer::NotifyMediaClientConnected(const std::string& conn_id, const std::string& stream_id, const std::string& visitor_device_id) {
        auto event = std::make_shared<GrPluginClientConnectedEvent>();
        event->conn_id_ = conn_id;
        event->stream_id_ = stream_id;
        event->conn_type_ = "Direct";
        event->visitor_device_id_ = visitor_device_id;
        event->begin_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
        this->plugin_->CallbackEvent(event);
        LOGI("Conn id: {}, device id: {}", stream_id, visitor_device_id);
    }

    void WsPluginServer::NotifyMediaClientDisConnected(const std::string& conn_id, const std::string& stream_id, const std::string& visitor_device_id, int64_t begin_timestamp) {
        auto event = std::make_shared<GrPluginClientDisConnectedEvent>();
        event->conn_id_ = conn_id;
        event->stream_id_ = stream_id;
        event->visitor_device_id_ = visitor_device_id;
        event->end_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
        event->duration_ = event->end_timestamp_ - begin_timestamp;
        this->plugin_->CallbackEvent(event);
    }

    int64_t WsPluginServer::GetQueuingMediaMsgCount() {
        int64_t count = 0;
        stream_routers_.ApplyAll([&](const auto&, const auto& r) {
            count += r->GetQueuingMsgCount();
        });
        return count;
    }

    int64_t WsPluginServer::GetQueuingFtMsgCount() {
        int count;
        ft_routers_.ApplyAll([&](const auto&, const auto& r) {
            count += r->GetQueuingMsgCount();
        });
        return count;
    }

    std::vector<std::shared_ptr<GrConnectedClientInfo>> WsPluginServer::GetConnectedClientInfo() {
        std::vector<std::shared_ptr<GrConnectedClientInfo>> clients_info;
        stream_routers_.VisitAll([&](const auto&, const std::shared_ptr<WsStreamRouter>& router) {
            clients_info.push_back(std::make_shared<GrConnectedClientInfo>(GrConnectedClientInfo {
                .device_id_ = router->visitor_device_id_,
                .stream_id_ = router->stream_id_,
                .device_name_ = router->device_name_,
            }));
        });
        return clients_info;
    }

    void WsPluginServer::OnClientHello(const std::shared_ptr<MsgClientHello>& event) {
        stream_routers_.VisitAll([&](const auto&, const std::shared_ptr<WsStreamRouter>& router) {
            LOGI("*** OnClientHello, evt stream id: {}, router stream id: {}, device name: {}",
                 event->stream_id_, router->stream_id_, event->device_name_);
            if (router->stream_id_ == event->stream_id_) {
                router->device_name_ = event->device_name_;
            }
        });
    }
}

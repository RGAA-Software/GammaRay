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

namespace tc
{

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

    WsPluginServer::WsPluginServer(tc::WsPlugin* plugin, uint16_t listen_port){
        this->plugin_ = plugin;
        this->listen_port_ = listen_port;
        http_handler_ = std::make_shared<HttpHandler>(plugin_);
    }

    void WsPluginServer::Start() {
        server_ = std::make_shared<asio2::https_server>();
        server_->bind_disconnect([=, this](std::shared_ptr<asio2::https_session>& sess_ptr) {
            auto socket_fd = (uint64_t)sess_ptr->socket().native_handle();
            LOGI("client disconnected: {}", socket_fd);
            if (stream_routers_.HasKey(socket_fd)) {
                if (auto opt_val = stream_routers_.Remove(socket_fd); opt_val.has_value()) {
                    const auto& val = opt_val.value();
                    NotifyMediaClientDisConnected(val->the_conn_id_, val->device_id_, val->created_timestamp_);
                    LOGI("client session removed: {}", val->device_id_);
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

        auto exe_dir = qApp->applicationDirPath().toStdString();
        auto pwd_file = std::format("{}/certs/password", exe_dir);
        auto pwd = (File::OpenForRead(pwd_file))->ReadAllAsString();
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

        server_->set_verify_mode(asio::ssl::verify_peer);

        // media websocket
        AddWebsocketRouter(kUrlMedia);
        AddWebsocketRouter(kUrlFileTransfer);

        // ping
        AddHttpRouter(kApiPing, [=, this](const std::string& path, http::web_request& req, http::web_response& rep) {
            http_handler_->HandlePing(req, rep);
        });

        if (listen_port_ <= 0) {
            LOGE("Listen port invalid: {}", listen_port_);
        }
        bool ret = server_->start("0.0.0.0", std::to_string(listen_port_));
        LOGI("App server start result: {}, listen port: {}", ret, listen_port_);
    }

    void WsPluginServer::Exit() {

    }

    void WsPluginServer::PostNetMessage(const std::string& data) {
        stream_routers_.ApplyAll([=](const uint64_t& socket_fd, const std::shared_ptr<WsStreamRouter>& router) {
//            if (!router->enable_video_) {
//                return;
//            }
            router->PostBinaryMessage(data);
        });
    }

    bool WsPluginServer::PostTargetStreamMessage(const std::string& stream_id, const std::string& data) {
        bool found_target_stream = false;
        stream_routers_.ApplyAll([=, &found_target_stream](const uint64_t& socket_fd, const std::shared_ptr<WsStreamRouter>& router) {
            if (stream_id == router->stream_id_) {
                router->PostBinaryMessage(data);
                found_target_stream = true;
            }
        });
        return found_target_stream;
    }

    bool WsPluginServer::PostTargetFileTransferMessage(const std::string& stream_id, const std::string& data) {
        bool found_target_stream = false;
        ft_routers_.ApplyAll([=, &found_target_stream](const uint64_t& socket_fd, const std::shared_ptr<WsFileTransferRouter>& router) {
            if (stream_id == router->stream_id_) {
                router->PostBinaryMessage(data);
                found_target_stream = true;
            }
        });
        return found_target_stream;
    }

    int WsPluginServer::GetConnectionPeerCount() {
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

    void WsPluginServer::AddWebsocketRouter(const std::string &path) {
        auto fn_get_socket_fd = [](std::shared_ptr<asio2::https_session> &sess_ptr) -> uint64_t {
            auto& s = sess_ptr->socket();
            return (uint64_t)s.native_handle();
        };
        server_->bind(path, websocket::listener<asio2::https_session>{}
            .on("message", [=, this](std::shared_ptr<asio2::https_session> &sess_ptr, std::string_view data) {
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
            .on("open", [=, this](std::shared_ptr<asio2::https_session> &sess_ptr) {
                auto query = sess_ptr->get_request().get_query();
                auto params = UrlHelper::ParseQueryString(std::string(query.data(), query.size()));
                for (const auto& [k, v] : params) {
                    LOGI("query param, k: {}, v: {}", k, v);
                }
                LOGI("App server {} open, query: {}", path, query);
                bool only_audio = std::atoi(params["only_audio"].c_str()) == 1;
                std::string device_id;
                std::string stream_id;
                if (params.contains("device_id")) {
                    device_id = params["device_id"];
                }
                if (params.contains("stream_id")) {
                    stream_id = params["stream_id"];
                }

                sess_ptr->set_no_delay(true);
                auto socket_fd = fn_get_socket_fd(sess_ptr);

                if (path == kUrlMedia) {
                    auto router = WsStreamRouter::Make(ws_data_, only_audio, device_id, stream_id);
                    stream_routers_.Insert(socket_fd, router);
                    NotifyMediaClientConnected(router->the_conn_id_, device_id);
                    router->OnOpen(sess_ptr);
                }
                else if (path == kUrlFileTransfer) {
                    auto router = WsFileTransferRouter::Make(ws_data_, only_audio, device_id, stream_id);
                    ft_routers_.Insert(socket_fd, router);
                    router->OnOpen(sess_ptr);
                }

            })
            .on("close", [=, this](std::shared_ptr<asio2::https_session> &sess_ptr) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                LOGI("client closed: {}", socket_fd);
                if (path == kUrlMedia) {
                    if (auto opt_val = stream_routers_.Remove(socket_fd); opt_val.has_value()) {
                        const auto& val = opt_val.value();
                        NotifyMediaClientDisConnected(val->the_conn_id_, val->device_id_, val->created_timestamp_);
                        LOGI("client session removed: {}", val->device_id_);
                    }
                }
                else if (path == kUrlFileTransfer) {
                    ft_routers_.Remove(socket_fd);
                    //NotifyMediaClientDisConnected();
                }
            })
            .on_ping([=, this](auto &sess_ptr) {

            })
            .on_pong([=, this](auto &sess_ptr) {

            })
        );
    }

    void WsPluginServer::AddHttpRouter(const std::string &path,
       std::function<void(const std::string& path, http::web_request& req, http::web_response& rep)>&& callback) {
        // bind it
        server_->bind<http::verb::get, http::verb::post>(path, [=, this](http::web_request &req, http::web_response &rep) {
            callback(path, req, rep);
        }, aop_log{}, http::enable_cache);
    }

    void WsPluginServer::NotifyMediaClientConnected(const std::string& the_conn_id, const std::string& device_id) {
        auto event = std::make_shared<GrPluginClientConnectedEvent>();
        event->the_conn_id_ = the_conn_id;
        event->conn_type_ = "Direct";
        event->device_id_ = device_id;
        event->begin_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
        this->plugin_->CallbackEvent(event);
        LOGI("Conn id: {},", the_conn_id);
    }

    void WsPluginServer::NotifyMediaClientDisConnected(const std::string& the_conn_id, const std::string& device_id, int64_t begin_timestamp) {
        auto event = std::make_shared<GrPluginClientDisConnectedEvent>();
        event->the_conn_id_ = the_conn_id;
        event->device_id_ = device_id;
        event->end_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
        event->duration_ = event->end_timestamp_ - begin_timestamp;
        this->plugin_->CallbackEvent(event);
    }

    int64_t WsPluginServer::GetQueuingMediaMsgCount() {
        int count;
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
}

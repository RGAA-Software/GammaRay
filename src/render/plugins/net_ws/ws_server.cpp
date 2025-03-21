//
// Created by RGAA on 2024/3/1.
//

#include "ws_server.h"

#include <memory>
#include "tc_common_new/log.h"
#include "tc_common_new/time_ext.h"
#include "tc_common_new/data.h"
#include "render/network/ws_media_router.h"
#include "ws_plugin_router.h"
#include "plugin_interface/gr_plugin_events.h"
#include "ws_plugin.h"
#include "tc_common_new/url_helper.h"
#include "http_handler.h"

static std::string kUrlMedia = "/media";
static std::string kApiPing = "/api/ping";

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
        http_server_ = std::make_shared<asio2::http_server>();
        http_server_->bind_disconnect([=, this](std::shared_ptr<asio2::http_session>& sess_ptr) {
            auto socket_fd = (uint64_t)sess_ptr->socket().native_handle();
            LOGI("client disconnected: {}", socket_fd);
            if (media_routers_.HasKey(socket_fd)) {
                media_routers_.Remove(socket_fd);
                NotifyMediaClientDisConnected();
                LOGI("App server media close, media router size: {}", media_routers_.Size());
            }
        });

        http_server_->support_websocket(true);
        ws_data_ = std::make_shared<WsData>(WsData{
            .vars_ = {
                {"plugin",  this->plugin_},
            }
        });
        // media websocket
        AddWebsocketRouter<std::shared_ptr<asio2::http_server>>(kUrlMedia, http_server_);

        // ping
        AddHttpRouter(kApiPing, [=, this](const std::string& path, http::web_request& req, http::web_response& rep) {
            http_handler_->HandlePing(req, rep);
        });

        if (listen_port_ <= 0) {
            LOGE("Listen port invalid: {}", listen_port_);
        }
        bool ret = http_server_->start("0.0.0.0", std::to_string(listen_port_));
        LOGI("App server start result: {}", ret);
    }

    void WsPluginServer::Exit() {

    }

    void WsPluginServer::PostNetMessage(const std::string& data) {
        media_routers_.ApplyAll([=](const uint64_t& socket_fd, const std::shared_ptr<WsPluginRouter>& router) {
            if (!router->enable_video_) {
                return;
            }
            router->PostBinaryMessage(data);
        });
    }

    bool WsPluginServer::PostTargetStreamMessage(const std::string& stream_id, const std::string& data) {
        bool found_target_stream = false;
        media_routers_.ApplyAll([=, &found_target_stream](const uint64_t& socket_fd, const std::shared_ptr<WsPluginRouter>& router) {
            if (stream_id == router->stream_id_) {
                router->PostBinaryMessage(data);
                found_target_stream = true;
            }
        });
        return found_target_stream;
    }

    int WsPluginServer::GetConnectionPeerCount() {
        return (int)media_routers_.Size();
    }

    bool WsPluginServer::IsOnlyAudioClients() {
        bool only_audio_client = true;
        media_routers_.VisitAllCond([&](auto k, auto& v) -> bool {
            if (v->enable_video_) {
                only_audio_client = false;
                return true;
            }
            return false;
        });
        return only_audio_client;
    }

    template<typename Server>
    void WsPluginServer::AddWebsocketRouter(const std::string &path, const Server &s) {
        auto fn_get_socket_fd = [](std::shared_ptr<asio2::http_session> &sess_ptr) -> uint64_t {
            auto& s = sess_ptr->socket();
            return (uint64_t)s.native_handle();
        };
        s->bind(path, websocket::listener<asio2::http_session>{}
            .on("message", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr, std::string_view data) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                if (path == kUrlMedia) {
                    media_routers_.VisitAll([=](auto k, auto &v) mutable {
                        if (socket_fd == k) {
                            v->OnMessage(sess_ptr, socket_fd, data);
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
                std::shared_ptr<WsPluginRouter> router = nullptr;
                if (path == kUrlMedia) {
                    router = WsPluginRouter::Make(ws_data_, only_audio, device_id, stream_id);
                    media_routers_.Insert(socket_fd, router);
                    NotifyMediaClientConnected();
                }

                if (router) {
                    router->OnOpen(sess_ptr);
                }
            })
            .on("close", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                LOGI("client closed: {}", socket_fd);
                if (path == kUrlMedia) {
                    media_routers_.Remove(socket_fd);
                    NotifyMediaClientDisConnected();
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
        http_server_->bind<http::verb::get, http::verb::post>(path, [=, this](http::web_request &req, http::web_response &rep) {
            callback(path, req, rep);
        }, aop_log{}, http::enable_cache);
    }

    void WsPluginServer::NotifyMediaClientConnected() {
        auto event = std::make_shared<GrPluginClientConnectedEvent>();
        this->plugin_->CallbackEvent(event);
    }

    void WsPluginServer::NotifyMediaClientDisConnected() {
        auto event = std::make_shared<GrPluginClientDisConnectedEvent>();
        this->plugin_->CallbackEvent(event);
    }
}

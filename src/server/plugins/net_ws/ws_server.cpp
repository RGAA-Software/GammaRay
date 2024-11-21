//
// Created by RGAA on 2024/3/1.
//

#include "ws_server.h"

#include <memory>
#include "tc_common_new/log.h"
#include "tc_common_new/time_ext.h"
#include "tc_common_new/data.h"
#include "server/network/ws_media_router.h"
//#include "message_processor.h"
//#include "http_handler.h"
#include "ws_plugin_router.h"

static std::string kUrlMedia = "/media";

static std::string kApiServerState = "/api/server/state";
static std::string kApiSdpRequest = "/api/sdp/request";

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

    WsPluginServer::WsPluginServer(tc::WsPlugin* plugin){
        this->plugin_ = plugin;
        //http_handler_ = std::make_shared<HttpHandler>(this->app_);
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
            this->NotifyPeerDisconnected();
        });

        http_server_->support_websocket(true);
        ws_data_ = std::make_shared<WsData>(WsData{
            .vars_ = {
                {"plugin",  this->plugin_},
            }
        });
        AddWebsocketRouter<std::shared_ptr<asio2::http_server>>(kUrlMedia, http_server_);
        AddHttpRouter<std::shared_ptr<asio2::http_server>>("/", http_server_);

        bool ret = http_server_->start("0.0.0.0", std::to_string(9090));
        LOGI("App server start result: {}", ret);
    }

    void WsPluginServer::Exit() {

    }

    void WsPluginServer::PostNetMessage(const std::string& data) {
        media_routers_.ApplyAll([=](const auto &k, const auto &v) {
            if (!v->enable_video_) {
                return;
            }
            v->PostBinaryMessage(data);
        });
    }

    int WsPluginServer::GetConnectionPeerCount() {
        return (int)media_routers_.Size();
    }

    void WsPluginServer::NotifyPeerConnected() {

    }

    void WsPluginServer::NotifyPeerDisconnected() {

    }

    bool WsPluginServer::OnlyAudioClient() {
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
                            v->OnMessage(sess_ptr, data);
                        }
                    });
                }
            })
            .on("open", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
                LOGI("App server {} open", path);

                //auto& s = sess_ptr->socket();
                //asio::error_code ec;
                //s.set_option(asio::ip::tcp::no_delay(false), ec);
                //s.set_option(asio::socket_base::send_buffer_size(1024*1024));
                //s.set_option(asio::socket_base::receive_buffer_size(64));
                //LOGI("NO DELAY EC: {}, msg: {}", ec.value(), ec.message());
                sess_ptr->set_no_delay(true);
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                std::shared_ptr<WsPluginRouter> router = nullptr;
                if (path == kUrlMedia) {
                    router = WsPluginRouter::Make(ws_data_);
                    media_routers_.Insert(socket_fd, router);
                    NotifyMediaClientConnected();
                }

                if (router) {
                    router->OnOpen(sess_ptr);
                }
                LOGI("After ws open");
                this->NotifyPeerConnected();
                LOGI("After notify peer connected.");
            })
            .on("close", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                LOGI("client closed: {}", socket_fd);
                if (path == kUrlMedia) {
                    media_routers_.Remove(socket_fd);
                    NotifyMediaClientDisConnected();
                }

                this->NotifyPeerDisconnected();
            })
            .on_ping([=, this](auto &sess_ptr) {

            })
            .on_pong([=, this](auto &sess_ptr) {

            })
        );
    }

    template<typename Server>
    void WsPluginServer::AddHttpRouter(const std::string &path, const Server &s) {
        s->bind<http::verb::get, http::verb::post>(path, [=, this](http::web_request &req, http::web_response &rep) {
            if (path == "/") {
                rep.fill_text("I'm working...");
            }
            else if (path == kApiServerState) {
                //http_handler_->HandleServerState(req, rep);
            }
            else if (path == kApiSdpRequest) {
                //http_handler_->HandleWebRtcSdpRequest(req, rep);
            }

        }, aop_log{}, http::enable_cache);
    }

    void WsPluginServer::NotifyMediaClientConnected() {
//        context_->SendAppMessage(MsgClientConnected {
//            .client_size_ = static_cast<int>(media_routers_.Size()),
//        });
    }

    void WsPluginServer::NotifyMediaClientDisConnected() {
//        context_->SendAppMessage(MsgClientDisconnected {
//            .client_size_ = static_cast<int>(media_routers_.Size()),
//        });
    }
}

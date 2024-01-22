//
// Created by hy on 2023/12/19.
//

#include "app_server.h"

#include "tc_common/log.h"
#include "http_handler.h"
#include "session.h"
#include "personal_data.h"

#ifdef WIN32
#pragma comment(lib, "uSockets.lib")
#pragma comment(lib, "uv.lib")
#pragma comment(lib, "zlib.lib")
#endif

namespace tc
{

    AppServer::AppServer(int port) {
        listening_port_ = port;

        http_handler_ = std::make_shared<HttpHandler>();
    }

    AppServer::~AppServer() {

    }

    void AppServer::Start() {
        server_thread_ = std::thread([this]() {
            this->StartInternal();
        });
    }

    void AppServer::Exit() {
        if (app_) {
            app_->close();
        }
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

    template<typename T>
    uWS::TemplatedApp<false>::WebSocketBehavior<T> AppServer::MakeMediaBehavior() {
        return uWS::TemplatedApp<false>::WebSocketBehavior<PerSocketData> {
                .open = [=, this](uWS::WebSocket<false, true, PerSocketData>* ws) {
                    this->AddSession(ws);
                },
                .message = [=, this](uWS::WebSocket<false, true, PerSocketData>* ws, std::string_view message, uWS::OpCode opCode) {
                    ws->send(message, opCode);
                    LOGI("WS.media message:{}", message);
                },
                .close = [=, this](uWS::WebSocket<false, true, PerSocketData>* ws, int /*code*/, std::string_view /*message*/) {
                    LOGI("WS.media close : {}", ws->getNativeHandle());
                    auto handle = reinterpret_cast<uint64_t>(ws->getNativeHandle());
                    this->RemoveSession(ws);
                }
        };
    }

    template<typename T>
    uWS::TemplatedApp<false>::WebSocketBehavior<T> AppServer::MakeIPCBehavior() {
        return uWS::TemplatedApp<false>::WebSocketBehavior<PerSocketData> {
                .open = [](uWS::WebSocket<false, true, PerSocketData>* ws) {
                    LOGI("WS.ipc open...{}", (void*)ws);
                },
                .message = [](uWS::WebSocket<false, true, PerSocketData>* ws, std::string_view message, uWS::OpCode opCode) {
                    ws->send(message, opCode);
                    LOGI("WS.ipc message:{}", message);
                },
                .close = [](uWS::WebSocket<false, true, PerSocketData>* ws, int /*code*/, std::string_view /*message*/) {
                    LOGI("WS.ipc close");
                }
        };
    }

    void AppServer::StartInternal() {
        app_ = std::make_shared<uWS::TemplatedApp<false>>(uWS::App());

        app_->get("/v1/supported/apis", std::bind(&HttpHandler::HandleSupportApis, http_handler_.get(), std::placeholders::_1, std::placeholders::_2))
            .post("/v1/report/info", std::bind(&HttpHandler::HandleReportInfo, http_handler_.get(), std::placeholders::_1, std::placeholders::_2))
            .ws<PerSocketData>("/media", MakeMediaBehavior<PerSocketData>())
            .ws<PerSocketData>("/ipc", MakeIPCBehavior<PerSocketData>())
            .listen(listening_port_, [this](us_listen_socket_t* socket) {
                if (socket) {
                    LOGI("App server listening on: {}", listening_port_);
                } else {
                    LOGE("App server start failed, check port or certs!");
                }
            }).run();
    }

    void AppServer::AddSession(WebsocketConn* conn) {
        std::lock_guard<std::mutex> guard(session_mtx_);
        sessions_.insert({
            reinterpret_cast<uint64_t>(conn->getNativeHandle()),
            Session::Make(conn)
        });
    }

    void AppServer::RemoveSession(WebsocketConn* conn) {
        std::lock_guard<std::mutex> guard(session_mtx_);
        auto handle = reinterpret_cast<uint64_t>(conn->getNativeHandle());
        sessions_.erase(handle);
    }

    void AppServer::BroadcastMessage(const std::string& msg) {
        std::lock_guard<std::mutex> guard(session_mtx_);
        for (const auto& [handle, session] : sessions_) {
            session->SendNetMessage(msg);
        }
    }

    SessionPair AppServer::GetSessions() {
        std::lock_guard<std::mutex> guard(session_mtx_);
        return sessions_;
    }
}

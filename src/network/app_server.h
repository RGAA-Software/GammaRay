//
// Created by hy on 2023/12/19.
//

#ifndef TC_APPLICATION_APP_SERVER_H
#define TC_APPLICATION_APP_SERVER_H

#include <thread>
#include <unordered_map>
#include <uwebsockets/App.h>

#include "session.h"
#include "app_messages.h"

namespace tc
{
    using SessionPair = std::unordered_map<uint64_t, std::shared_ptr<Session>>;

    class HttpHandler;

    class AppServer {
    public:

        explicit AppServer(int port);
        ~AppServer();

        void Start();
        void Exit();

        void BroadcastMessage(const std::string& msg);
        SessionPair GetSessions();

    private:
        void StartInternal();
        void AddSession(WebsocketConn* conn);
        void RemoveSession(WebsocketConn* conn);

        template<typename T>
        uWS::TemplatedApp<false>::WebSocketBehavior<T> MakeMediaBehavior();

        template<typename T>
        uWS::TemplatedApp<false>::WebSocketBehavior<T> MakeIPCBehavior();

    private:

        int listening_port_ = 0;
        std::thread server_thread_;
        std::shared_ptr<uWS::TemplatedApp<false>> app_ = nullptr;
        std::shared_ptr<HttpHandler> http_handler_ = nullptr;

        std::mutex session_mtx_;
        SessionPair sessions_;

    };

}

#endif //TC_APPLICATION_APP_SERVER_H

//
// Created by RGAA on 2024/3/1.
//

#ifndef TC_APPLICATION_APP_SERVER_H
#define TC_APPLICATION_APP_SERVER_H

#include <memory>
#include "connection.h"
#include "ws_router.h"
#include "tc_common_new/concurrent_hashmap.h"
#include <asio2/asio2.hpp>

namespace tc
{
    class Application;
    class HttpHandler;

    class AppServer : public Connection {
    public:

        explicit AppServer(const std::shared_ptr<Application>& app);

        void Start() override;
        void Exit() override;

        void PostVideoMessage(const std::string& data) override;
        void PostAudioMessage(const std::string& data) override;
        void PostControlMessage(const std::string& data) override;
        void PostIpcMessage(const std::string& msg) override;
        int GetConnectionPeerCount() override;
        void NotifyPeerConnected() override;
        void NotifyPeerDisconnected() override;
        bool OnlyAudioClient() override;

    private:

        template<typename Server>
        void AddWebsocketRouter(const std::string& path, const Server& s);

        template<typename Server>
        void AddHttpRouter(const std::string& path, const Server& s);

        void NotifyMediaClientConnected();
        void NotifyMediaClientDisConnected();

    private:

        std::shared_ptr<asio2::http_server> http_server_ = nullptr;

        WsDataPtr ws_data_ = nullptr;
        tc::ConcurrentHashMap<uint64_t, WsRouterPtr> media_routers_;
        tc::ConcurrentHashMap<uint64_t, WsRouterPtr> control_routers_;
        tc::ConcurrentHashMap<uint64_t, WsRouterPtr> ipc_routers_;

        std::shared_ptr<HttpHandler> http_handler_ = nullptr;

    };
}

#endif //TC_APPLICATION_APP_SERVER_H

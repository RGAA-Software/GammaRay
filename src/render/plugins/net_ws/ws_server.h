//
// Created by RGAA on 2024/3/1.
//

#ifndef TC_APPLICATION_APP_SERVER_H
#define TC_APPLICATION_APP_SERVER_H

#include <memory>
#include "render/network/ws_router.h"
#include "tc_common_new/concurrent_hashmap.h"
#include <asio2/asio2.hpp>

namespace tc
{
    class WsPluginRouter;
    class HttpHandler;
    class WsPlugin;

    class WsPluginServer {
    public:

        explicit WsPluginServer(tc::WsPlugin* plugin, uint16_t listen_port);

        void Start();
        void Exit();

        void PostNetMessage(const std::string& data);
        int GetConnectionPeerCount();
        void NotifyPeerConnected();
        void NotifyPeerDisconnected();
        bool IsOnlyAudioClients();

    private:

        template<typename Server>
        void AddWebsocketRouter(const std::string& path, const Server& s);

        template<typename Server>
        void AddHttpRouter(const std::string& path, const Server& s);

        void NotifyMediaClientConnected();
        void NotifyMediaClientDisConnected();

    private:
        tc::WsPlugin* plugin_ = nullptr;
        uint16_t listen_port_ = 0;
        std::shared_ptr<asio2::http_server> http_server_ = nullptr;

        WsDataPtr ws_data_ = nullptr;
        tc::ConcurrentHashMap<uint64_t, std::shared_ptr<WsPluginRouter>> media_routers_;

        //std::shared_ptr<HttpHandler> http_handler_ = nullptr;

    };
}

#endif //TC_APPLICATION_APP_SERVER_H

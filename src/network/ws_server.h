//
// Created by RGAA on 2024-03-30.
//

#ifndef TC_SERVER_STEAM_WS_SERVER_H
#define TC_SERVER_STEAM_WS_SERVER_H

#include <memory>
#include <asio2/websocket/ws_server.hpp>

#include "tc_common_new/concurrent_hashmap.h"

namespace tc
{
    class GrContext;

    class WSServer {
    public:

        static std::shared_ptr<WSServer> Make(const std::shared_ptr<GrContext>& ctx);
        explicit WSServer(const std::shared_ptr<GrContext>& ctx);
        ~WSServer();

        void Start();
        void Exit();

        void PostBinaryMessage(const std::string& msg);
        void PostBinaryMessage(std::string_view msg);
        void ParseBinaryMessage(std::string_view msg);

    private:
        std::shared_ptr<asio2::ws_server> server_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        ConcurrentHashMap<uint64_t, std::shared_ptr<asio2::ws_session>> sessions_;
    };
}

#endif //TC_SERVER_STEAM_WS_SERVER_H

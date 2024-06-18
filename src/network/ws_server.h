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

    class WSSession {
    public:
        uint64_t socket_fd_;
        int session_type_;
        std::shared_ptr<asio2::ws_session> session_ = nullptr;
    };

    class WSServer {
    public:

        static std::shared_ptr<WSServer> Make(const std::shared_ptr<GrContext>& ctx);
        explicit WSServer(const std::shared_ptr<GrContext>& ctx);
        ~WSServer();

        void Start();
        void Exit();

        void PostBinaryMessage(const std::string& msg, bool only_inner = false);
        void PostBinaryMessage(std::string_view msg, bool only_inner = false);
        void ParseBinaryMessage(uint64_t socket_fd, std::string_view msg);

    private:
        std::shared_ptr<asio2::ws_server> server_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        ConcurrentHashMap<uint64_t, std::shared_ptr<WSSession>> sessions_;
    };
}

#endif //TC_SERVER_STEAM_WS_SERVER_H

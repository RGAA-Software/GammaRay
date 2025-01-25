//
// Created by RGAA on 25/01/2025.
//

#ifndef GAMMARAY_WS_SIG_CLIENT_H
#define GAMMARAY_WS_SIG_CLIENT_H

#include <memory>
#include <string>
#include <atomic>
#include <asio2/websocket/ws_client.hpp>

namespace tc
{

    class GrContext;
    class GrApplication;
    class GrStatistics;
    class MessageListener;

    class WsSigClient {
    public:
        explicit WsSigClient(const std::shared_ptr<GrApplication>& app);
        void Start();
        void Exit();
        bool IsAlive();
        void PostNetMessage(const std::string& msg);

    private:
        void HeartBeat();
        void ParseMessage(const std::string& msg);

    private:
        GrStatistics* statistics_ = nullptr;
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<asio2::ws_client> client_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::atomic_int queued_msg_count_ = 0;
    };

}

#endif //GAMMARAY_WS_SIG_CLIENT_H

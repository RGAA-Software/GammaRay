//
// Created by RGAA on 2024-04-20.
//

#ifndef GAMMARAY_WS_PANEL_CLIENT_H
#define GAMMARAY_WS_CLIENT_H

#include <memory>
#include <string>
#include <atomic>
#include <asio2/websocket/wss_client.hpp>

namespace tc
{

    class GrContext;
    class GrApplication;
    class GrStatistics;
    class MessageListener;

    class GrServiceClient {
    public:
        explicit GrServiceClient(const std::shared_ptr<GrApplication>& app);
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
        std::atomic_int queuing_message_count_ = 0;
    };

}

#endif //GAMMARAY_WS_PANEL_CLIENT_H

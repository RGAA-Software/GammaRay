//
// Created by RGAA on 2024-04-20.
//

#ifndef GAMMARAY_RENDER_SERVICE_CLIENT_H
#define GAMMARAY_RENDER_SERVICE_CLIENT_H

#include <memory>
#include <string>
#include <atomic>
#include <asio2/websocket/wss_client.hpp>

namespace tc
{

    class RdContext;
    class RdApplication;
    class RdStatistics;
    class MessageListener;

    class RenderServiceClient {
    public:

        explicit RenderServiceClient(const std::shared_ptr<RdApplication>& app);
        void Start();
        void Exit();
        bool IsAlive();
        void PostNetMessage(const std::string& msg);

    private:
        void HeartBeat();
        void ParseMessage(const std::string& msg);

    private:
        RdStatistics* statistics_ = nullptr;
        std::shared_ptr<RdApplication> app_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<asio2::wss_client> client_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::atomic_int queuing_message_count_ = 0;
    };

}

#endif //GAMMARAY_WS_CLIENT_H

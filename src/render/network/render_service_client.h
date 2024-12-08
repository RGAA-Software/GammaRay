//
// Created by RGAA on 2024-04-20.
//

#ifndef GAMMARAY_RENDER_SERVICE_CLIENT_H
#define GAMMARAY_RENDER_SERVICE_CLIENT_H

#include <memory>
#include <string>
#include <atomic>
#include <asio2/websocket/ws_client.hpp>

namespace tc
{

    class Context;
    class Application;
    class Statistics;
    class MessageListener;

    class RenderServiceClient {
    public:

        explicit RenderServiceClient(const std::shared_ptr<Application>& app);
        void Start();
        void Exit();
        bool IsAlive();
        void PostNetMessage(const std::string& msg);

    private:
        void HeartBeat();
        void ParseMessage(const std::string& msg);

    private:
        Statistics* statistics_ = nullptr;
        std::shared_ptr<Application> app_ = nullptr;
        std::shared_ptr<Context> context_ = nullptr;
        std::shared_ptr<asio2::ws_client> client_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::atomic_int queued_msg_count_ = 0;
    };

}

#endif //GAMMARAY_WS_CLIENT_H

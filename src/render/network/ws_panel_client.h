//
// Created by RGAA on 2024-04-20.
//

#ifndef GAMMARAY_WS_PANEL_CLIENT_H
#define GAMMARAY_WS_PANEL_CLIENT_H

#include <memory>
#include <string>
#include <atomic>
#include <asio2/websocket/ws_client.hpp>

namespace tc
{

    class Context;
    class Statistics;
    class MessageListener;
    class Settings;

    class WsPanelClient {
    public:
        explicit WsPanelClient(const std::shared_ptr<Context>& ctx);
        void Start();
        void Exit();
        void PostNetMessage(const std::string& msg);

    private:
        void SendStatistics();
        void ParseNetMessage(std::string_view msg);

    private:
        Statistics* statistics_ = nullptr;
        Settings* settings_ = nullptr;
        std::shared_ptr<Context> context_ = nullptr;
        std::shared_ptr<asio2::ws_client> client_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::atomic_int queued_msg_count_ = 0;
    };

}

#endif //GAMMARAY_WS_PANEL_CLIENT_H

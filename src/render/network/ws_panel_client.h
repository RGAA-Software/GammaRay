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

    class RdContext;
    class RdStatistics;
    class MessageListener;
    class RdSettings;
    class PluginManager;

    class WsPanelClient {
    public:
        explicit WsPanelClient(const std::shared_ptr<RdContext>& ctx);
        void Start();
        void Exit();
        void PostNetMessage(const std::string& msg);

    private:
        void SendStatistics();
        void SendPluginsInfo();
        void ParseNetMessage(std::string_view msg);
        void ProcessCommandEnablePlugin(const std::string& plugin_id);
        void ProcessCommandDisablePlugin(const std::string& plugin_id);

    private:
        RdStatistics* statistics_ = nullptr;
        RdSettings* settings_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<asio2::ws_client> client_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::atomic_int queuing_message_count_ = 0;
        std::shared_ptr<PluginManager> plugin_mgr_ = nullptr;
    };

}

#endif //GAMMARAY_WS_PANEL_CLIENT_H

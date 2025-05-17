//
// Created by RGAA on 17/05/2025.
//

#ifndef GAMMARAY_CT_PANEL_CLIENT_H
#define GAMMARAY_CT_PANEL_CLIENT_H

#include <memory>
#include <asio2/websocket/ws_client.hpp>

namespace tc
{

    class ClientContext;

    class CtPanelClient {
    public:
        explicit CtPanelClient(const std::shared_ptr<ClientContext>& ctx);
        void Start();

    private:
        bool IsAlive();
        void ParseMessage(std::string_view data);
        void HeartBeat();

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<asio2::ws_client> client_ = nullptr;
    };

}

#endif //GAMMARAY_CT_PANEL_CLIENT_H

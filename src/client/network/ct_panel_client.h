//
// Created by RGAA on 17/05/2025.
//

#ifndef GAMMARAY_CT_PANEL_CLIENT_H
#define GAMMARAY_CT_PANEL_CLIENT_H

#include <memory>
#include "ct_app_message.h"
#include <asio2/websocket/wss_client.hpp>

namespace tc
{

    class ClientContext;
    class MessageListener;

    class CtPanelClient {
    public:
        explicit CtPanelClient(const std::shared_ptr<ClientContext>& ctx);
        void Start();

    private:
        bool IsAlive();
        void ParseMessage(std::string_view data);
        void Hello();
        void HeartBeat();
        void ReportFileTransferBegin(const MsgClientFileTransmissionBegin& msg);
        void ReportFileTransferEnd(const MsgClientFileTransmissionEnd& msg);

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<asio2::wss_client> client_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };

}

#endif //GAMMARAY_CT_PANEL_CLIENT_H

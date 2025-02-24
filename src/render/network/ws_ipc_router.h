//
// Created by RGAA on 2024/3/5.
//

#ifndef TC_APPLICATION_WS_IPC_ROUTER_H
#define TC_APPLICATION_WS_IPC_ROUTER_H

#include "ws_router.h"

namespace tc
{

    class Data;

    class WsIpcRouter : public WsRouter {
    public:

        static std::shared_ptr<WsIpcRouter> Make(const WsDataPtr& data) {
            return std::make_shared<WsIpcRouter>(data);
        }

        explicit WsIpcRouter(const WsDataPtr& data) : WsRouter(data) {}
        void OnOpen(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnClose(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnMessage(std::shared_ptr<asio2::http_session> &sess_ptr, int64_t socket_fd, std::string_view data) override;
        void OnPing(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnPong(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void PostBinaryMessage(const std::shared_ptr<Data> &data) override;
        void PostBinaryMessage(const std::string &data) override;

    };

}

#endif //TC_APPLICATION_WS_MEDIA_ROUTER_H

//
// Created by RGAA on 2024/3/5.
//

#ifndef TC_APPLICATION_WS_MEDIA_ROUTER_H
#define TC_APPLICATION_WS_MEDIA_ROUTER_H

#include "ws_router.h"

namespace tc
{

    class Data;

    class WsMediaRouter : public WsRouter, public std::enable_shared_from_this<WsMediaRouter> {
    public:

        static std::shared_ptr<WsMediaRouter> Make(const WsDataPtr& data) {
            return std::make_shared<WsMediaRouter>(data);
        }

        explicit WsMediaRouter(const WsDataPtr& data) : WsRouter(data) {}
        void OnOpen(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnClose(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnMessage(std::shared_ptr<asio2::http_session> &sess_ptr, int64_t socket_fd, std::string_view data) override;
        void OnPing(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnPong(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void PostBinaryMessage(std::shared_ptr<Data> data) override;
        void PostBinaryMessage(const std::string &data) override;
        void PostTextMessage(const std::string& data) override;
    };

}

#endif //TC_APPLICATION_WS_MEDIA_ROUTER_H

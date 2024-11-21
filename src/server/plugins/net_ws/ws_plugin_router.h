//
// Created by RGAA on 2024/3/5.
//

#ifndef TC_APPLICATION_WS_PLUGIN_ROUTER_H
#define TC_APPLICATION_WS_PLUGIN_ROUTER_H

#include "server/network/ws_router.h"

namespace tc
{

    class Data;

    class WsPluginRouter : public WsRouter, public std::enable_shared_from_this<WsPluginRouter> {
    public:

        static std::shared_ptr<WsPluginRouter> Make(const WsDataPtr& data) {
            return std::make_shared<WsPluginRouter>(data);
        }

        explicit WsPluginRouter(const WsDataPtr& data) : WsRouter(data) {}
        void OnOpen(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnClose(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnMessage(std::shared_ptr<asio2::http_session> &sess_ptr, std::string_view data) override;
        void OnPing(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnPong(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void PostBinaryMessage(const std::shared_ptr<Data> &data) override;
        void PostBinaryMessage(const std::string &data) override;

    public:
        bool enable_video_ = true;

    };

}

#endif //TC_APPLICATION_WS_MEDIA_ROUTER_H

//
// Created by RGAA on 2024/3/5.
//

#ifndef TC_APPLICATION_WS_PLUGIN_ROUTER_H
#define TC_APPLICATION_WS_PLUGIN_ROUTER_H

#include "render/network/ws_router.h"
//#include "render/network/wss_router.h"

namespace tc
{

    class Data;

    class WsStreamRouter : public WsRouter, public std::enable_shared_from_this<WsStreamRouter> {
    public:
        static std::shared_ptr<WsStreamRouter> Make(const WsDataPtr& data, bool only_audio, const std::string& visitor_device_id, const std::string& stream_id) {
            auto router = std::make_shared<WsStreamRouter>(data, only_audio);
            router->visitor_device_id_ = visitor_device_id;
            router->stream_id_ = stream_id;
            return router;
        }

        explicit WsStreamRouter(const WsDataPtr& data, bool only_audio) : WsRouter(data), enable_video_(!only_audio) {}
        void OnOpen(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnClose(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnMessage(std::shared_ptr<asio2::http_session> &sess_ptr, int64_t socket_fd, std::string_view data) override;
        void OnPing(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void OnPong(std::shared_ptr<asio2::http_session> &sess_ptr) override;
        void PostBinaryMessage(std::shared_ptr<Data> data) override;
        void PostBinaryMessage(const std::string &data) override;
        void PostTextMessage(const std::string& data) override;

    public:
        bool enable_video_ = true;
        std::string visitor_device_id_;
        std::string stream_id_;
        unsigned int post_thread_id_ = 0;
        std::string device_name_;
    };

}

#endif //TC_APPLICATION_WS_MEDIA_ROUTER_H

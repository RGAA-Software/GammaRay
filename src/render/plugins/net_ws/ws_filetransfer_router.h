//
// Created by RGAA on 2024/3/5.
//

#ifndef TC_PLUGIN_WS_FILE_TRANSFER_ROUTER_H
#define TC_PLUGIN_WS_FILE_TRANSFER_ROUTER_H

#include "render/network/wss_router.h"

namespace tc
{

    class Data;

    class WsFileTransferRouter : public WssRouter, public std::enable_shared_from_this<WsFileTransferRouter> {
    public:

        static std::shared_ptr<WsFileTransferRouter> Make(const WsDataPtr& data, bool only_audio, const std::string& device_id, const std::string& stream_id) {
            auto router = std::make_shared<WsFileTransferRouter>(data, only_audio);
            router->device_id_ = device_id;
            router->stream_id_ = stream_id;
            return router;
        }

        explicit WsFileTransferRouter(const WsDataPtr& data, bool only_audio) : WssRouter(data){}
        void OnOpen(std::shared_ptr<asio2::https_session> &sess_ptr) override;
        void OnClose(std::shared_ptr<asio2::https_session> &sess_ptr) override;
        void OnMessage(std::shared_ptr<asio2::https_session> &sess_ptr, int64_t socket_fd, std::string_view data) override;
        void OnPing(std::shared_ptr<asio2::https_session> &sess_ptr) override;
        void OnPong(std::shared_ptr<asio2::https_session> &sess_ptr) override;
        void PostBinaryMessage(std::shared_ptr<Data> msg) override;

    public:
        std::string device_id_;
        std::string stream_id_;
        unsigned int post_thread_id_ = 0;
    };

}

#endif //TC_APPLICATION_WS_MEDIA_ROUTER_H

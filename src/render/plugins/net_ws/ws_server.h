//
// Created by RGAA on 2024/3/1.
//

#ifndef TC_APPLICATION_APP_SERVER_H
#define TC_APPLICATION_APP_SERVER_H

#include <memory>
#include <vector>
#include "render/network/ws_router.h"
#include "tc_common_new/concurrent_hashmap.h"
#include <asio2/asio2.hpp>

namespace tc
{
    class WsStreamRouter;
    class WsFileTransferRouter;
    class HttpHandler;
    class WsPlugin;
    class GrConnectedClientInfo;
    class MsgClientHello;

    class WsPluginServer {
    public:

        explicit WsPluginServer(tc::WsPlugin* plugin, uint16_t listen_port);

        void Start();
        void Exit();

        void PostNetMessage(const std::string& data);
        bool PostTargetStreamMessage(const std::string& stream_id, const std::string& data);
        bool PostTargetFileTransferMessage(const std::string& stream_id, const std::string& data);
        int GetConnectedClientsCount();
        bool IsOnlyAudioClients();
        bool IsWorking();
        int64_t GetQueuingMediaMsgCount();
        int64_t GetQueuingFtMsgCount();
        std::vector<std::shared_ptr<GrConnectedClientInfo>> GetConnectedClientInfo();
        void OnClientHello(const std::shared_ptr<MsgClientHello>& event);

    private:
        void AddWebsocketRouter(const std::string& path);

        void AddHttpRouter(const std::string& path,
                           std::function<void(const std::string& path, http::web_request& req, http::web_response& rep)>&& callback);

        void NotifyMediaClientConnected(const std::string& stream_id, const std::string& visitor_device_id);
        void NotifyMediaClientDisConnected(const std::string& stream_id, const std::string& visitor_device_id, int64_t begin_timestamp);

    private:
        tc::WsPlugin* plugin_ = nullptr;
        uint16_t listen_port_ = 0;
        std::shared_ptr<asio2::https_server> server_ = nullptr;

        WsDataPtr ws_data_ = nullptr;
        tc::ConcurrentHashMap<uint64_t, std::shared_ptr<WsStreamRouter>> stream_routers_;
        tc::ConcurrentHashMap<uint64_t, std::shared_ptr<WsFileTransferRouter>> ft_routers_;

        std::shared_ptr<HttpHandler> http_handler_ = nullptr;

    };
}

#endif //TC_APPLICATION_APP_SERVER_H

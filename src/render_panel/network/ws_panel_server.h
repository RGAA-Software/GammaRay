//
// Created by RGAA on 2024-03-30.
//

#ifndef TC_SERVER_STEAM_WS_SERVER_H
#define TC_SERVER_STEAM_WS_SERVER_H

#ifndef ASIO2_ENABLE_SSL
#define ASIO2_ENABLE_SSL
#endif

#include <memory>
#include <asio2/asio2.hpp>
#include "tc_common_new/concurrent_hashmap.h"
#include "render/network/ws_router.h"

namespace tc
{
    class Data;
    class GrContext;
    class GrApplication;
    class HttpHandler;
    class GrSettings;
    class FileTransferChannel;
    class VisitRecordOperator;
    class FileTransferRecordOperator;
    class MessageListener;

    class WSSession {
    public:
        uint64_t socket_fd_;
        int session_type_;
        std::shared_ptr<asio2::http_session> session_ = nullptr;
        std::string stream_id_;
    };

    class FtSession : public WSSession {
    public:
        std::shared_ptr<FileTransferChannel> ch_ = nullptr;
    };

    class WsPanelServer {
    public:
        static std::shared_ptr<WsPanelServer> Make(const std::shared_ptr<GrApplication>& app);
        explicit WsPanelServer(const std::shared_ptr<GrApplication>& ctx);
        ~WsPanelServer();

        void Start();
        void Exit();
        bool IsAlive();

        // to /panel socket
        void PostPanelMessage(const std::string& msg, bool only_inner = false);

        // parse /panel socket
        bool ParsePanelMessage(uint64_t socket_fd, std::string_view msg);

        // to /panel/renderer socket
        void PostRendererMessage(std::shared_ptr<Data> msg);

        // parse /panel/renderer socket
        void ParseRendererMessage(uint64_t socket_fd, std::string_view msg);

        void ParseFtBinaryMessage(uint64_t socket_fd, std::string_view msg);

    private:
        void AddWebsocketRouter(const std::string& path);

        void AddHttpGetRouter(const std::string& path,
           std::function<void(const std::string& path, http::web_request &req, http::web_response &rep)>&& cbk);

        void AddHttpPostRouter(const std::string& path,
           std::function<void(const std::string& path, http::web_request &req, http::web_response &rep)>&& cbk);

        void RpSyncPanelInfo();

    private:
        std::shared_ptr<asio2::http_server> server_ = nullptr;
        WsDataPtr ws_data_ = nullptr;
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        ConcurrentHashMap<uint64_t, std::shared_ptr<WSSession>> panel_sessions_;
        ConcurrentHashMap<uint64_t, std::shared_ptr<WSSession>> renderer_sessions_;
        ConcurrentHashMap<uint64_t, std::shared_ptr<FtSession>> ft_sessions_;
        std::shared_ptr<HttpHandler> http_handler_ = nullptr;
        GrSettings* settings_ = nullptr;
        std::shared_ptr<VisitRecordOperator> visit_record_op_ = nullptr;
        std::shared_ptr<FileTransferRecordOperator> ft_record_op_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };
}

#endif //TC_SERVER_STEAM_WS_SERVER_H

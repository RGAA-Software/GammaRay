//
// Created by hy on 8/07/2024.
//

#ifndef GAMMARAY_FILETRANSFER_H
#define GAMMARAY_FILETRANSFER_H

#include <memory>
#include <asio2/websocket/ws_server.hpp>

namespace tc
{

    class GrContext;
    class GrSettings;

    class FileTransferChannel {
    public:
        static std::shared_ptr<FileTransferChannel> Make(const std::shared_ptr<GrContext>& ctx);
        explicit FileTransferChannel(const std::shared_ptr<GrContext>& ctx);
        void Start();
        void Exit();

    private:
        void ParseMessage(std::string_view data);
        void PostBinaryMessage(const std::string& msg);

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<asio2::ws_server> server_ = nullptr;
        std::shared_ptr<asio2::ws_session> session_ = nullptr;
    };

}

#endif //GAMMARAY_FILETRANSFER_H

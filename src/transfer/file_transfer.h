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

    class FileTransfer {
    public:
        static std::shared_ptr<FileTransfer> Make(const std::shared_ptr<GrContext>& ctx);
        explicit FileTransfer(const std::shared_ptr<GrContext>& ctx);
        void Start();
        void Exit();

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<asio2::ws_server> server_ = nullptr;
    };

}

#endif //GAMMARAY_FILETRANSFER_H

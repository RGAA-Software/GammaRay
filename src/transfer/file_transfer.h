//
// Created by hy on 8/07/2024.
//

#ifndef GAMMARAY_FILETRANSFER_H
#define GAMMARAY_FILETRANSFER_H

#include <memory>

namespace tc
{

    class GrContext;
    class GrSettings;
    class WSServer;

    class FileTransfer {
    public:
        static std::shared_ptr<FileTransfer> Make(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<WSServer>& server);
        explicit FileTransfer(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<WSServer>& server);
        void Start();
        void Exit();

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<WSServer> ws_server_ = nullptr;
    };

}

#endif //GAMMARAY_FILETRANSFER_H

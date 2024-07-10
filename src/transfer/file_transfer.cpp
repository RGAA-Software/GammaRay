//
// Created by hy on 8/07/2024.
//

#include "file_transfer.h"
#include "gr_settings.h"
#include "gr_context.h"
#include "network/ws_server.h"

namespace tc
{

    std::shared_ptr<FileTransfer> FileTransfer::Make(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<WSServer>& server) {
        return std::make_shared<FileTransfer>(ctx, server);
    }

    FileTransfer::FileTransfer(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<WSServer>& server) {
        context_ = ctx;
        settings_ = GrSettings::Instance();
        ws_server_ = server;
    }

    void FileTransfer::Start() {

    }

    void FileTransfer::Exit() {

    }

}

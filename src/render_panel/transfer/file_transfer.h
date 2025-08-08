//
// Created by RGAA  on 8/07/2024.
//

#ifndef GAMMARAY_FILETRANSFER_H
#define GAMMARAY_FILETRANSFER_H
#ifndef ASIO2_ENABLE_SSL
#define ASIO2_ENABLE_SSL
#endif
#include <memory>
#include <asio2/asio2.hpp>

namespace tc
{

    class GrContext;
    class GrSettings;
    class Thread;
    class File;

    // @Deprecated !!
    // Drag & Drop file transferring
    class FileTransferChannel {
    public:
        explicit FileTransferChannel(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<asio2::http_session>& sess);
        void OnConnected();
        void OnDisConnected();
        void ParseBinaryMessage(std::string_view data);
        void PostBinaryMessage(const std::string& msg);

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<File> transferring_file_ = nullptr;
        std::shared_ptr<asio2::http_session> sess_ = nullptr;
    };

}

#endif //GAMMARAY_FILETRANSFER_H

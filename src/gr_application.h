//
// Created by RGAA on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_GR_APPLICATION_H
#define TC_SERVER_STEAM_GR_APPLICATION_H

#include <memory>
#include <QTimer>
#include <QObject>

namespace tc
{

    class GrContext;
    class GameModel;
    class HttpServer;
    class WSServer;
    class UdpBroadcaster;
    class GrServerManager;
    class GrSettings;
    class GrSystemMonitor;
    class FileTransfer;

    class GrApplication : public QObject, public std::enable_shared_from_this<GrApplication> {
    public:

        GrApplication();
        ~GrApplication() override;

        void Init();

        std::shared_ptr<GrContext> GetContext() { return context_; }
        std::shared_ptr<WSServer> GetWSServer() { return ws_server_; }

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<HttpServer> http_server_ = nullptr;
        std::shared_ptr<WSServer> ws_server_ = nullptr;
        std::shared_ptr<UdpBroadcaster> udp_broadcaster_ = nullptr;
        std::shared_ptr<GrSystemMonitor> sys_monitor_ = nullptr;
        std::shared_ptr<FileTransfer> file_transfer_ = nullptr;
        QTimer* timer_ = nullptr;
        GrSettings* settings_ = nullptr;

    };

}

#endif //TC_SERVER_STEAM_GR_APPLICATION_H

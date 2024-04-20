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
    class ServerManager;
    class GrSettings;
    class SystemMonitor;

    class GrApplication : public QObject, public std::enable_shared_from_this<GrApplication> {
    public:

        GrApplication();
        ~GrApplication() override;

        void Init();

        std::shared_ptr<GrContext> GetContext() { return context_; }

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<HttpServer> http_server_ = nullptr;
        std::shared_ptr<WSServer> ws_server_ = nullptr;
        std::shared_ptr<UdpBroadcaster> udp_broadcaster_ = nullptr;
        std::shared_ptr<SystemMonitor> sys_monitor_ = nullptr;

        QTimer* timer_ = nullptr;
        GrSettings* settings_ = nullptr;

    };

}

#endif //TC_SERVER_STEAM_GR_APPLICATION_H

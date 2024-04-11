//
// Created by hy on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_APPLICATION_H
#define TC_SERVER_STEAM_APPLICATION_H

#include <memory>
#include <QTimer>
#include <QObject>

namespace tc
{

    class Context;
    class GameModel;
    class HttpServer;
    class WSServer;
    class UdpBroadcaster;
    class AppManager;
    class Settings;

    class Application : public QObject, public std::enable_shared_from_this<Application> {
    public:

        Application();
        ~Application() override;

        void Init();

        std::shared_ptr<Context> GetContext() { return context_; }
        std::shared_ptr<AppManager> GetAppManager() { return app_manager_; }

    private:

    private:

        std::shared_ptr<Context> context_ = nullptr;
        //GameModel* installed_game_model_ = nullptr;

        std::shared_ptr<HttpServer> http_server_ = nullptr;
        std::shared_ptr<WSServer> ws_server_ = nullptr;
        std::shared_ptr<UdpBroadcaster> udp_broadcaster_ = nullptr;
        std::shared_ptr<AppManager> app_manager_ = nullptr;

        QTimer* timer_ = nullptr;

        Settings* settings_ = nullptr;

    };

}

#endif //TC_SERVER_STEAM_APPLICATION_H

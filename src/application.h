//
// Created by hy on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_APPLICATION_H
#define TC_SERVER_STEAM_APPLICATION_H

#include <memory>

namespace tc
{

    class Context;
    class GameModel;
    class HttpServer;

    class Application {
    public:

        Application();
        ~Application();

        void Init();
        GameModel* GetInstalledModel();

    private:

        std::shared_ptr<Context> context_ = nullptr;
        GameModel* installed_game_model_ = nullptr;

        std::shared_ptr<HttpServer> http_server_ = nullptr;

    };

}

#endif //TC_SERVER_STEAM_APPLICATION_H

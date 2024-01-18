//
// Created by hy on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_APPLICATION_H
#define TC_SERVER_STEAM_APPLICATION_H

#include <memory>

namespace tc
{

    class Context;
    class SteamManager;

    class Application {
    public:

        Application();

        void Init();

    private:

        std::shared_ptr<Context> context_ = nullptr;
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;

    };

}

#endif //TC_SERVER_STEAM_APPLICATION_H

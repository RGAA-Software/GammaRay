//
// Created by hy on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_CONTEXT_H
#define TC_SERVER_STEAM_CONTEXT_H

#include <memory>

namespace tc
{

    class SteamManager;

    class Context : public std::enable_shared_from_this<Context> {
    public:
        Context();

        void Init();
        std::shared_ptr<SteamManager> GetSteamManager();

    private:
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;

    };

}
#endif //TC_SERVER_STEAM_CONTEXT_H

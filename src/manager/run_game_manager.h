//
// Created by hy on 2024/4/26.
//

#ifndef GAMMARAY_RUN_GAME_MANAGER_H
#define GAMMARAY_RUN_GAME_MANAGER_H

#include <memory>

namespace tc
{

    class GrContext;
    class SteamManager;
    class DBGameManager;

    class RunGameManager {
    public:

        explicit RunGameManager(const std::shared_ptr<GrContext>& ctx);
        ~RunGameManager();


    private:
        std::shared_ptr<GrContext> gr_ctx_ = nullptr;
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;
        std::shared_ptr<DBGameManager> db_game_manager_ = nullptr;
    };

}

#endif //GAMMARAY_RUN_GAME_MANAGER_H

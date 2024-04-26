//
// Created by hy on 2024/4/26.
//

#include "run_game_manager.h"

#include "gr_context.h"
#include "tc_steam_manager_new/steam_manager.h"

namespace tc
{

    RunGameManager::RunGameManager(const std::shared_ptr<GrContext>& ctx) {
        this->gr_ctx_ = ctx;
        this->steam_mgr_ = ctx->GetSteamManager();
        this->db_game_manager_ = ctx->GetDBGameManager();
    }

    RunGameManager::~RunGameManager() {

    }

}

//
// Created by hy on 2024/1/17.
//

#include "context.h"

#include "steam/steam_manager.h"

namespace tc
{

    Context::Context() {

    }

    void Context::Init() {
        steam_mgr_ = SteamManager::Make(shared_from_this());
        steam_mgr_->ScanInstalledGames();
        steam_mgr_->DumpGamesInfo();
    }

    std::shared_ptr<SteamManager> Context::GetSteamManager() {
        return steam_mgr_;
    }

}
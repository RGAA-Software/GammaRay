//
// Created by hy on 2024/1/17.
//

#include "context.h"

#include "steam/steam_manager.h"
#include "tc_common/task_runtime.h"

namespace tc
{

    Context::Context() {

    }

    void Context::Init() {
        task_runtime_ = std::make_shared<TaskRuntime>();

        // last...
        steam_mgr_ = SteamManager::Make(shared_from_this());
        steam_mgr_->ScanInstalledGames();
        steam_mgr_->DumpGamesInfo();
        steam_mgr_->UpdateAppDetails();
    }

    std::shared_ptr<SteamManager> Context::GetSteamManager() {
        return steam_mgr_;
    }

    std::shared_ptr<TaskRuntime> Context::GetTaskRuntime() {
        return task_runtime_;
    }

}
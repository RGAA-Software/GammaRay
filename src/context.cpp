//
// Created by hy on 2024/1/17.
//

#include "context.h"

#include "tc_common_new/task_runtime.h"
#include "tc_steam_manager_new/steam_manager.h"

namespace tc
{

    Context::Context() {

    }

    void Context::Init() {
        task_runtime_ = std::make_shared<TaskRuntime>();

        steam_mgr_ = SteamManager::Make(task_runtime_);
        steam_mgr_->ScanInstalledGames();
        steam_mgr_->DumpGamesInfo();
        //steam_mgr_->UpdateAppDetails();
    }

    std::shared_ptr<SteamManager> Context::GetSteamManager() {
        return steam_mgr_;
    }

    std::shared_ptr<TaskRuntime> Context::GetTaskRuntime() {
        return task_runtime_;
    }

}
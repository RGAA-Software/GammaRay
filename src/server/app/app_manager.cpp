//
// Created by RGAA on 2023-12-20.
//

#include "app_manager.h"
#include "context.h"
#include "tc_common_new/log.h"
#include "tc_steam_manager_new/steam_manager.h"

namespace tc
{

    AppManager::AppManager(const std::shared_ptr<Context>& ctx) {
        context_ = ctx;
    }

    AppManager::~AppManager() {

    }

    void AppManager::Init() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        //InitSteamManager();
    }

    bool AppManager::StartProcess() {
        return true;
    }

    bool AppManager::StartProcessWithHook() {
        return true;
    }

#if 0
    void AppManager::InitSteamManager() {
        // steam manager
        steam_manager_ = SteamManager::Make(context_->GetTaskRuntime());
        steam_manager_->ScanInstalledGames();
    }
#endif

    void AppManager::Exit() {

    }

    void AppManager::CloseCurrentApp() {

    }

}
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
    }

    bool AppManager::StartProcess() {
        return true;
    }

    bool AppManager::StartProcessWithHook() {
        return true;
    }

    void AppManager::Exit() {

    }

    void AppManager::CloseCurrentApp() {

    }

}
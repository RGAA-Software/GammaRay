//
// Created by hy on 2024/1/17.
//

#include "application.h"

#include "context.h"
#include "steam/steam_manager.h"

namespace tc
{

    Application::Application() {

    }

    void Application::Init() {
        context_ = std::make_shared<Context>();

        steam_mgr_ = SteamManager::Make(context_);
        steam_mgr_->Init();


    }

}

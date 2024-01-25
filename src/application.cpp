//
// Created by hy on 2024/1/17.
//

#include "application.h"

#include "context.h"
#include "steam/steam_manager.h"
#include "model/game_model.h"
#include "network/http_server.h"

namespace tc
{

    Application::Application() {

    }

    Application::~Application() {
        delete installed_game_model_;
    }

    void Application::Init() {
        context_ = std::make_shared<Context>();
        context_->Init();

        http_server_ = std::make_shared<HttpServer>(context_);
        http_server_->Start();

        installed_game_model_ = new GameModel();

        for (auto& game : context_->GetSteamManager()->GetInstalledGames()) {
            installed_game_model_->AddGame(game);
        }
    }

    GameModel* Application::GetInstalledModel() {
        return installed_game_model_;
    }

}

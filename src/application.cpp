//
// Created by hy on 2024/1/17.
//

#include "application.h"

#include "context.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "model/game_model.h"
#include "network/http_server.h"
#include "network/ws_server.h"
#include "network/udp_broadcaster.h"
#include "app/tc_app_manager.h"

#include <QTimer>

namespace tc
{

    Application::Application() : QObject(nullptr) {

    }

    Application::~Application() {
        delete installed_game_model_;
    }

    void Application::Init() {
        context_ = std::make_shared<Context>();
        context_->Init();

        app_manager_ = std::make_shared<AppManager>(context_);

        http_server_ = std::make_shared<HttpServer>(context_);
        http_server_->Start();

        ws_server_ = WSServer::Make(context_);
        ws_server_->Start();

        udp_broadcaster_ = UdpBroadcaster::Make(context_);

        installed_game_model_ = new GameModel();

        for (auto& game : context_->GetSteamManager()->GetInstalledGames()) {
            installed_game_model_->AddGame(game);
        }

        timer_ = new QTimer(this);
        connect(timer_, &QTimer::timeout, this, [=, this]() {
            udp_broadcaster_->Broadcast("this message is from udp...");
        });
        timer_->start(1000);

        app_manager_->Start("");
    }

    GameModel* Application::GetInstalledModel() {
        return installed_game_model_;
    }

}

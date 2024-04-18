//
// Created by hy on 2024/1/17.
//

#include "gr_application.h"

#include "gr_context.h"
#include "gr_settings.h"
#include "network/ws_server.h"
#include "network/http_server.h"
#include "manager/tc_app_manager.h"
#include "network/udp_broadcaster.h"
#include "tc_3rdparty/json/json.hpp"
#include "tc_steam_manager_new/steam_manager.h"

#include <QTimer>

using namespace nlohmann;

namespace tc
{

    GrApplication::GrApplication() : QObject(nullptr) {

    }

    GrApplication::~GrApplication() {
    }

    void GrApplication::Init() {
        settings_ = GrSettings::Instance();
        settings_->Load();

        context_ = std::make_shared<GrContext>();
        context_->Init();

        app_manager_ = std::make_shared<AppManager>(context_);

        http_server_ = std::make_shared<HttpServer>(shared_from_this());
        http_server_->Start();

        ws_server_ = WSServer::Make(context_);
        ws_server_->Start();

        udp_broadcaster_ = UdpBroadcaster::Make(context_);

        auto broadcast_msg = context_->MakeBroadcastMessage();
        timer_ = new QTimer(this);
        connect(timer_, &QTimer::timeout, this, [=, this]() {
            udp_broadcaster_->Broadcast(broadcast_msg);
        });
        timer_->start(1000);

    }


}

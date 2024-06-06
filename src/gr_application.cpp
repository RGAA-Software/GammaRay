//
// Created by RGAA on 2024/1/17.
//

#include "gr_application.h"

#include "gr_context.h"
#include "gr_settings.h"
#include "network/ws_server.h"
#include "network/http_server.h"
#include "network/udp_broadcaster.h"
#include "tc_3rdparty/json/json.hpp"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_common_new/shared_preference.h"
#include "system_monitor.h"
#include "gr_statistics.h"
#include "util/qt_directory.h"

#include <QTimer>
#include <QApplication>

using namespace nlohmann;

namespace tc
{

    GrApplication::GrApplication() : QObject(nullptr) {

    }

    GrApplication::~GrApplication() {
    }

    void GrApplication::Init() {
        SharedPreference::Instance()->Init("", "game.dat");
        settings_ = GrSettings::Instance();
        settings_->Load();
        settings_->Dump();

        auto exeDir = QApplication::applicationDirPath().toStdString();
        QtDirectory::CreateDir(std::format("{}/clients/windows", exeDir));
        QtDirectory::CreateDir(std::format("{}/clients/android", exeDir));

        context_ = std::make_shared<GrContext>();
        context_->Init();

        auto st = GrStatistics::Instance();
        st->SetContext(context_);
        st->RegisterEventListeners();

        http_server_ = std::make_shared<HttpServer>(shared_from_this());
        http_server_->Start();

        ws_server_ = WSServer::Make(context_);
        ws_server_->Start();

        sys_monitor_ = SystemMonitor::Make(shared_from_this());
        sys_monitor_->Start();

        udp_broadcaster_ = UdpBroadcaster::Make(context_);

        auto broadcast_msg = context_->MakeBroadcastMessage();
        timer_ = new QTimer(this);
        connect(timer_, &QTimer::timeout, this, [=, this]() {
            udp_broadcaster_->Broadcast(broadcast_msg);
        });
        timer_->start(1000);

    }


}

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
#include "gr_system_monitor.h"
#include "gr_statistics.h"
#include "util/qt_directory.h"
#include "transfer/file_transfer.h"
#include "tc_common_new/win32/firewall_helper.h"
#include "tc_common_new/log.h"

#include <QTimer>
#include <QApplication>

using namespace nlohmann;

namespace tc
{

    GrApplication::GrApplication() : QObject(nullptr) {

    }

    GrApplication::~GrApplication() = default;

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

        // register firewall
        auto app_path = qApp->applicationDirPath() + "/" + kGammaRayName.c_str();
        auto srv_path = qApp->applicationDirPath() + "/" + kGammaRayServerName.c_str();
        auto fh = FirewallHelper::Instance();
        fh->AddProgramToFirewall(RulesInfo("GammaRayIn", app_path.toStdString(), "", 1));
        fh->AddProgramToFirewall(RulesInfo("GammaRayOut", app_path.toStdString(), "", 2));
        fh->AddProgramToFirewall(RulesInfo("GammaRayServerIn", srv_path.toStdString(), "", 1));
        fh->AddProgramToFirewall(RulesInfo("GammaRayServerOut", srv_path.toStdString(), "", 2));

        LOGI("app path: {}", app_path.toStdString());
        LOGI("srv path: {}", srv_path.toStdString());

        auto st = GrStatistics::Instance();
        st->SetContext(context_);
        st->RegisterEventListeners();

        http_server_ = std::make_shared<HttpServer>(shared_from_this());
        http_server_->Start();

        ws_server_ = WSServer::Make(context_);
        ws_server_->Start();

        sys_monitor_ = GrSystemMonitor::Make(shared_from_this());
        sys_monitor_->Start();

        udp_broadcaster_ = UdpBroadcaster::Make(context_);

        auto broadcast_msg = context_->MakeBroadcastMessage();
        timer_ = new QTimer(this);
        connect(timer_, &QTimer::timeout, this, [=, this]() {
            udp_broadcaster_->Broadcast(broadcast_msg);
        });
        timer_->start(1000);

        file_transfer_ = FileTransferChannel::Make(context_);
        file_transfer_->Start();

    }

    void GrApplication::Exit() {
        context_->Exit();
    }


}

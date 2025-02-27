//
// Created by RGAA on 2024/1/17.
//

#include "gr_application.h"

#include "gr_context.h"
#include "gr_settings.h"
#include "gr_statistics.h"
#include "gr_system_monitor.h"
#include "gr_app_messages.h"
#include "tc_common_new/folder_util.h"
#include "tc_common_new/log.h"
#include "tc_common_new/win32/firewall_helper.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/message_notifier.h"
#include "render_panel/network/ws_panel_server.h"
#include "render_panel/network/udp_broadcaster.h"
#include "render_panel/network/gr_service_client.h"
#include "render_panel/network/ws_sig_client.h"
#include "tc_3rdparty/json/json.hpp"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_manager_client/mgr_client_sdk.h"
#include "tc_manager_client/mgr_device_operator.h"
#include "tc_manager_client/mgr_device.h"

#include <QTimer>
#include <QApplication>

using namespace nlohmann;

namespace tc
{

    GrApplication::GrApplication() : QObject(nullptr) {

    }

    GrApplication::~GrApplication() = default;

    void GrApplication::Init() {
        auto sp_dir = qApp->applicationDirPath() + "/gr_data";
        SharedPreference::Instance()->Init(sp_dir.toStdString(), "gammaray.dat");
        settings_ = GrSettings::Instance();
        settings_->Load();
        settings_->Dump();

        auto exeDir = QApplication::applicationDirPath().toStdString();
        FolderUtil::CreateDir(std::format("{}/clients/windows", exeDir));
        FolderUtil::CreateDir(std::format("{}/clients/android", exeDir));

        context_ = std::make_shared<GrContext>();
        context_->Init(shared_from_this());

        // register firewall
        auto app_path = qApp->applicationDirPath() + "/" + kGammaRayName.c_str();
        auto srv_path = qApp->applicationDirPath() + "/" + kGammaRayRenderName.c_str();
        auto client_path = qApp->applicationDirPath() + "/" + kGammaRayClient.c_str();
        auto client_inner_path = qApp->applicationDirPath() + "/" + kGammaRayClientInner.c_str();
        auto fh = FirewallHelper::Instance();
        fh->RemoveProgramFromFirewall("GammaRayIn");
        fh->RemoveProgramFromFirewall("GammaRayOut");
        fh->RemoveProgramFromFirewall("GammaRayServerIn");
        fh->RemoveProgramFromFirewall("GammaRayServerOut");
        fh->RemoveProgramFromFirewall("GammaRayClientIn");
        fh->RemoveProgramFromFirewall("GammaRayClientOut");
        fh->RemoveProgramFromFirewall("GammaRayClientInnerIn");
        fh->RemoveProgramFromFirewall("GammaRayClientInnerOut");

        fh->AddProgramToFirewall(RulesInfo("GammaRayIn", app_path.toStdString(), "", 1));
        fh->AddProgramToFirewall(RulesInfo("GammaRayOut", app_path.toStdString(), "", 2));
        fh->AddProgramToFirewall(RulesInfo("GammaRayServerIn", srv_path.toStdString(), "", 1));
        fh->AddProgramToFirewall(RulesInfo("GammaRayServerOut", srv_path.toStdString(), "", 2));
        fh->AddProgramToFirewall(RulesInfo("GammaRayClientIn", app_path.toStdString(), "", 1));
        fh->AddProgramToFirewall(RulesInfo("GammaRayClientOut", app_path.toStdString(), "", 2));
        fh->AddProgramToFirewall(RulesInfo("GammaRayClientInnerIn", app_path.toStdString(), "", 1));
        fh->AddProgramToFirewall(RulesInfo("GammaRayClientInnerOut", app_path.toStdString(), "", 2));

        LOGI("app path: {}", app_path.toStdString());
        LOGI("srv path: {}", srv_path.toStdString());

        auto st = GrStatistics::Instance();
        st->SetContext(context_);
        st->RegisterEventListeners();

        mgr_client_sdk_ = std::make_shared<MgrClientSdk>(context_->GetMessageNotifier());

        ws_panel_server_ = WsPanelServer::Make(shared_from_this());
        ws_panel_server_->Start();

        sys_monitor_ = GrSystemMonitor::Make(shared_from_this());
        sys_monitor_->Start();

        udp_broadcaster_ = UdpBroadcaster::Make(context_);

        auto broadcast_msg = context_->MakeBroadcastMessage();
        timer_ = new QTimer(this);
        connect(timer_, &QTimer::timeout, this, [=, this]() {
            udp_broadcaster_->Broadcast(broadcast_msg);
        });
        timer_->start(1000);

        service_client_ = std::make_shared<GrServiceClient>(shared_from_this());
        service_client_->Start();

        sig_client_ = std::make_shared<WsSigClient>(shared_from_this());
        sig_client_->Start();

        RefreshSigServerSettings();
        RegisterMessageListener();
    }

    void GrApplication::Exit() {
        context_->Exit();
    }

    bool GrApplication::PostMessage2Service(const std::string& msg) {
        if (!service_client_ || !service_client_->IsAlive()) {
            return false;
        }
        service_client_->PostNetMessage(msg);
        return true;
    }

    void GrApplication::RefreshSigServerSettings() {
        mgr_client_sdk_->SetSdkParam(MgrClientSdkParam {
            .host_ = settings_->mgr_server_address_,
            .port_ = std::atoi(settings_->mgr_server_port_.c_str()),
            .ssl_ = false,
        });
    }

    void GrApplication::RegisterMessageListener() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgSettingsChanged>([=, this](const MsgSettingsChanged& msg) {
            RefreshSigServerSettings();
            RequestNewClientId(false);
        });
    }

    void GrApplication::RequestNewClientId(bool force_update) {
        if (!force_update && !settings_->device_id_.empty() && !settings_->device_random_pwd_.empty()) {
            return;
        }

        context_->PostTask([=, this]() {
            auto device = mgr_client_sdk_->GetDeviceOperator()->RequestNewDevice("");
            if (!device) {
                LOGE("Can't create new device!");
                return;
            }

            settings_->SetDeviceId(device->device_id_);
            settings_->SetDeviceRandomPwd(device->random_pwd_);

            context_->SendAppMessage(MsgRequestedNewDevice {
                .device_id_ = device->device_id_,
                .device_random_pwd_ = device->random_pwd_,
                .force_update_ = force_update,
            });

        });
    }

}

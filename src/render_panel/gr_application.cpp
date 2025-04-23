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
#include "tc_common_new/time_util.h"
#include "gr_account_manager.h"
#include "tc_dialog.h"

#include <QTimer>
#include <QApplication>

using namespace nlohmann;

namespace tc
{

    std::shared_ptr<GrApplication> grApp;

    GrApplication::GrApplication() : QObject(nullptr) {

    }

    GrApplication::~GrApplication() = default;

    void GrApplication::Init() {
        TimeDuration td("GrApplication::Init");
        grApp = shared_from_this();
        msg_notifier_ = std::make_shared<MessageNotifier>();

        auto begin_ctx_init_ts = TimeUtil::GetCurrentTimestamp();
        settings_ = GrSettings::Instance();
        settings_->Init(msg_notifier_);
        settings_->Load();
        settings_->Dump();

        auto exeDir = QApplication::applicationDirPath().toStdString();
        FolderUtil::CreateDir(std::format("{}/clients/windows", exeDir));
        FolderUtil::CreateDir(std::format("{}/clients/android", exeDir));

        context_ = std::make_shared<GrContext>();
        context_->Init(shared_from_this());
        auto ctx_init_diff = TimeUtil::GetCurrentTimestamp() - begin_ctx_init_ts;
        LOGI("** Context init used: {}ms", ctx_init_diff);

        account_mgr_ = std::make_shared<GrAccountManager>(context_);

        // firewall
        context_->PostTask([this]() {
            this->RegisterFirewall();
        });

        auto begin_conn_ts = TimeUtil::GetCurrentTimestamp();
        auto st = GrStatistics::Instance();
        st->SetContext(context_);
        st->RegisterEventListeners();

        mgr_client_sdk_ = std::make_shared<MgrClientSdk>(context_->GetMessageNotifier());

        ws_panel_server_ = WsPanelServer::Make(shared_from_this());
        ws_panel_server_->Start();

        sys_monitor_ = GrSystemMonitor::Make(shared_from_this());
        sys_monitor_->Start();

        //udp_broadcaster_ = UdpBroadcaster::Make(context_);

        service_client_ = std::make_shared<GrServiceClient>(shared_from_this());
        service_client_->Start();

        sig_client_ = std::make_shared<WsSigClient>(shared_from_this());
        sig_client_->Start();

        auto conn_diff = TimeUtil::GetCurrentTimestamp() - begin_conn_ts;
        LOGI("** Connection used: {}ms", conn_diff);

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
            .host_ = settings_->profile_server_host_,
            .port_ = std::atoi(settings_->profile_server_port_.c_str()),
            .ssl_ = false,
        });
    }

    void GrApplication::RegisterMessageListener() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgSettingsChanged>([=, this](const MsgSettingsChanged& msg) {
            RefreshSigServerSettings();
            RequestNewClientId(false);
        });

        msg_listener_->Listen<MsgGrTimer5S>([=, this](const MsgGrTimer5S& msg) {
            if (settings_->device_id_.empty()) {
                RequestNewClientId(false);
            }
        });

        msg_listener_->Listen<MsgForceRequestDeviceId>([=, this](const MsgForceRequestDeviceId& msg) {
            RequestNewClientId(true);
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

            context_->SendAppMessage(MsgSyncSettingsToRender{});

        });
    }

    void GrApplication::RegisterFirewall() {
        // register firewall
        auto begin_fm_ts = TimeUtil::GetCurrentTimestamp();
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
        auto fm_diff = TimeUtil::GetCurrentTimestamp()-begin_fm_ts;
        LOGI("** Firewall init used: {}ms", fm_diff);
        LOGI("app path: {}", app_path.toStdString());
        LOGI("srv path: {}", srv_path.toStdString());
    }

    std::shared_ptr<MessageNotifier> GrApplication::GetMessageNotifier() {
        return msg_notifier_;
    }

    bool GrApplication::CheckLocalDeviceInfoWithPopup() {
        auto r = account_mgr_->IsDeviceInfoOk();
        auto ok = r.value_or(false);
        if (ok) {
            return true;
        }

        auto err_msg = GrAccountError2String(r.error());
        auto dlg = TcDialog::Make(tr("Error"), std::format("Local device info error: {}", err_msg).c_str(), nullptr);
        dlg->show();
        return false;
    }
}

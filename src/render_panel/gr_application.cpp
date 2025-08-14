//
// Created by RGAA on 2024/1/17.
//

#include "gr_application.h"
#include <QTimer>
#include <QScreen>
#include <QApplication>
#include "tc_dialog.h"
#include "gr_workspace.h"
#include "tc_label.h"
#include "gr_context.h"
#include "gr_settings.h"
#include "gr_statistics.h"
#include "gr_app_messages.h"
#include "tc_common_new/log.h"
#include "gr_system_monitor.h"
#include "gr_account_manager.h"
#include "gr_connected_manager.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/time_util.h"
#include "ui/input_safety_pwd_dialog.h"
#include "tc_manager_client/mgr_device.h"
#include "tc_3rdparty/json/json.hpp"
#include "tc_relay_client/relay_api.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_manager_client/mgr_client_sdk.h"
#include "tc_manager_client/mgr_device_operator.h"
#include "tc_common_new/folder_util.h"
#include "tc_common_new/http_client.h"
#include "tc_common_new/win32/firewall_helper.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/message_notifier.h"
#include "render_panel/gr_guard_starter.h"
#include "render_panel/database/stream_item.h"
#include "render_panel/gr_render_msg_processor.h"
#include "render_panel/network/ws_panel_server.h"
#include "render_panel/network/udp_broadcaster.h"
#include "render_panel/network/gr_service_client.h"
#include "render_panel/devices/stream_messages.h"
#include "render_panel/system/win/win_panel_message_loop.h"
#include "render_panel/clipboard/panel_clipboard_manager.h"

#include <shellapi.h>

using namespace nlohmann;

namespace tc
{

    std::shared_ptr<GrApplication> grApp;

    GrApplication::GrApplication(QWidget* main_window, bool run_automatically) : QObject(main_window) {
        main_window_ = main_window;
        run_automatically_ = run_automatically;
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

        context_ = std::make_shared<GrContext>(main_window_);
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

        gr_connected_manager_ = std::make_shared<GrConnectedManager>(context_);
        rd_msg_processor_ = std::make_shared<GrRenderMsgProcessor>(context_);
        clipboard_mgr_ = std::make_shared<ClipboardManager>(context_);
        guard_starter_ = std::make_shared<GrGuardStarter>(context_);
        QCoreApplication::instance()->installNativeEventFilter(gr_connected_manager_.get());

        auto conn_diff = TimeUtil::GetCurrentTimestamp() - begin_conn_ts;
        LOGI("** Connection used: {}ms", conn_diff);

        RefreshSigServerSettings();
        RegisterMessageListener();
        StartWindowsMessagesLooping();

        if (!run_automatically_) {
            context_->PostUIDelayTask([=, this]() {
                this->UpdateServerSecurityPasswordIfNeeded();
                CheckSecurityPassword();
            }, 500);
        }
    }

    void GrApplication::Exit() {
        if (win_msg_loop_) {
            win_msg_loop_->Stop();
        }
        if (win_msg_thread_ && win_msg_thread_->IsJoinable()) {
            win_msg_thread_->Join();
        }
        context_->Exit();
    }

    bool GrApplication::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) {
        if(eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
        {
            MSG* pMsg = reinterpret_cast<MSG*>(message);
            if(pMsg->message == WM_COPYDATA) {

            }
            else if(pMsg->message == WM_DROPFILES) {
                LOGI("DROP FILES.......");
                HDROP hDrop = (HDROP)pMsg->wParam;
                UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0); // 获取文件数量

                for (UINT i = 0; i < fileCount; i++) {
                    char filePath[MAX_PATH];
                    DragQueryFileA(hDrop, i, filePath, MAX_PATH); // 获取文件路径
                    LOGI("===> DROP: {}", filePath);
                }

                DragFinish(hDrop); // 释放资源
            }
        }
        return false;
    }

    bool GrApplication::IsServiceConnected() {
        return service_client_ && service_client_->IsAlive();
    }

    bool GrApplication::PostMessage2Service(const std::string& msg) {
        if (!IsServiceConnected()) {
            return false;
        }
        service_client_->PostNetMessage(msg);
        return true;
    }

    bool GrApplication::IsRendererConnected() {
        return ws_panel_server_ && ws_panel_server_->IsAlive();
    }

    bool GrApplication::PostMessage2Renderer(std::shared_ptr<Data> msg) {
        if (!IsRendererConnected()) {
            return false;
        }
        ws_panel_server_->PostRendererMessage(msg);
        return true;
    }

    void GrApplication::RefreshSigServerSettings() {
        mgr_client_sdk_->SetSdkParam(MgrClientSdkParam {
            .host_ = settings_->GetSpvrServerHost(),
            .port_ = settings_->GetSpvrServerPort(),
            .ssl_ = false,
        });
    }

    void GrApplication::RegisterMessageListener() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgSettingsChanged>([=, this](const MsgSettingsChanged& msg) {
            LOGI("Settings changed...");
            RefreshSigServerSettings();
            bool force_update = settings_->GetDeviceId().empty();
            RequestNewClientId(force_update);
        });

        msg_listener_->Listen<MsgGrTimer5S>([=, this](const MsgGrTimer5S& msg) {
            if (settings_->GetDeviceId().empty()) {
                RequestNewClientId(true);
            }
        });

        msg_listener_->Listen<MsgForceRequestDeviceId>([=, this](const MsgForceRequestDeviceId& msg) {
            RequestNewClientId(true);
        });
    }

    void GrApplication::RequestNewClientId(bool force_update) {
        if (!force_update && !settings_->GetDeviceId().empty() && !settings_->GetDeviceRandomPwd().empty()) {
            return;
        }

        context_->PostTask([=, this]() {
            if (!settings_->HasSpvrServerConfig()) {
                return;
            }
            auto device = mgr_client_sdk_->GetDeviceOperator()->RequestNewDevice("");
            if (!device) {
                LOGE("Can't create new device!");
                return;
            }

            settings_->SetDeviceId(device->device_id_);
            settings_->SetDeviceRandomPwd(device->gen_random_pwd_);

            context_->SendAppMessage(MsgRequestedNewDevice {
                .device_id_ = device->device_id_,
                .device_random_pwd_ = device->gen_random_pwd_,
                .force_update_ = force_update,
            });

            context_->SendAppMessage(MsgSyncSettingsToRender{});

        });
    }

    void GrApplication::RegisterFirewall() {
        // register firewall
        auto begin_fm_ts = TimeUtil::GetCurrentTimestamp();
        auto app_path = qApp->applicationDirPath() + "/" + kGammaRayName.c_str();
        auto render_path = qApp->applicationDirPath() + "/" + kGammaRayRenderName.c_str();
        auto client_inner_path = qApp->applicationDirPath() + "/" + kGammaRayClientInner.c_str();
        auto guard_path = qApp->applicationDirPath() + "/" + kGammaRayGuardName.c_str();
        auto service_path = qApp->applicationDirPath() + "/" + kGammaRayService.c_str();
        auto fh = FirewallHelper::Instance();

        fh->RemoveProgramFromFirewall("GammaRayIn");
        fh->RemoveProgramFromFirewall("GammaRayOut");
        fh->RemoveProgramFromFirewall("GammaRayRenderIn");
        fh->RemoveProgramFromFirewall("GammaRayRenderOut");
        fh->RemoveProgramFromFirewall("GammaRayClientInnerIn");
        fh->RemoveProgramFromFirewall("GammaRayClientInnerOut");
        fh->RemoveProgramFromFirewall("GammaRayGuardIn");
        fh->RemoveProgramFromFirewall("GammaRayGuardOut");
        fh->RemoveProgramFromFirewall("GammaRayServiceIn");
        fh->RemoveProgramFromFirewall("GammaRayServiceOut");

        fh->AddProgramToFirewall(RulesInfo("GammaRayIn", app_path.toStdString(), "", 1));
        fh->AddProgramToFirewall(RulesInfo("GammaRayOut", app_path.toStdString(), "", 2));
        fh->AddProgramToFirewall(RulesInfo("GammaRayRenderIn", render_path.toStdString(), "", 1));
        fh->AddProgramToFirewall(RulesInfo("GammaRayRenderOut", render_path.toStdString(), "", 2));
        fh->AddProgramToFirewall(RulesInfo("GammaRayClientInnerIn", client_inner_path.toStdString(), "", 1));
        fh->AddProgramToFirewall(RulesInfo("GammaRayClientInnerOut", client_inner_path.toStdString(), "", 2));
        fh->AddProgramToFirewall(RulesInfo("GammaRayGuardIn", guard_path.toStdString(), "", 1));
        fh->AddProgramToFirewall(RulesInfo("GammaRayGuardOut", guard_path.toStdString(), "", 2));
        fh->AddProgramToFirewall(RulesInfo("GammaRayServiceIn", service_path.toStdString(), "", 1));
        fh->AddProgramToFirewall(RulesInfo("GammaRayServiceOut", service_path.toStdString(), "", 2));
        auto fm_diff = TimeUtil::GetCurrentTimestamp()-begin_fm_ts;
        LOGI("** Firewall init used: {}ms", fm_diff);
        LOGI("app path: {}", app_path.toStdString());
        LOGI("render path: {}", render_path.toStdString());
        LOGI("client inner path: {}", client_inner_path.toStdString());
        LOGI("client inner path: {}", guard_path.toStdString());
        LOGI("client inner path: {}", service_path.toStdString());
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
        QString pre_msg = tcTr("id_local_device_info_error");
        TcDialog dialog(tcTr("id_error"), pre_msg + std::format(" {}", err_msg).c_str(), grWorkspace.get());
        dialog.exec();
        return false;
    }

    std::shared_ptr<MgrClientSdk> GrApplication::GetManagerClient() {
        return mgr_client_sdk_;
    }

    std::shared_ptr<MgrDeviceOperator> GrApplication::GetDeviceOperator() {
        return mgr_client_sdk_->GetDeviceOperator();
    }

    void GrApplication::CheckSecurityPassword() {
        if (settings_->GetDeviceSecurityPwd().empty()) {
            InputSafetyPwdDialog dialog(grApp, grWorkspace.get());
            dialog.exec();
        }
    }

    void GrApplication::UpdateServerSecurityPasswordIfNeeded() {
        context_->PostTask([this]() {
            if (settings_->GetDeviceSecurityPwd().empty()) {
                return;
            }

            auto dev_opt = GetDeviceOperator();
            dev_opt->QueryDevice("");
        });
    }

    void GrApplication::StartWindowsMessagesLooping() {
        win_msg_thread_ = std::make_shared<Thread>([=, this]() {
            win_msg_loop_ = std::make_shared<WinMessageLoop>(shared_from_this());
            win_msg_loop_->Start();
        }, "", false);
    }

    bool GrApplication::PostMessage2RemoteRender(const std::shared_ptr<GrBaseStreamMessage>& msg) {
        if (!msg || !msg->stream_item_) {
            return false;
        }

        auto& item = msg->stream_item_;
        if (item->IsRelay()) {
            auto srv_remote_device_id = "server_" + item->remote_device_id_;
            auto res = relay::RelayApi::NotifyEvent(item->stream_host_, item->stream_port_, context_->GetDeviceIdOrIpAddress(), srv_remote_device_id, msg->AsJson());
            if (res.has_value()) {
                if (res.value() == relay::kRelayOk) {
                    return true;
                }
                else {
                    LOGE("NotifyEvent failed, res: {}", res.value());
                }
            }
            return false;
        }
        else {
            // host & port mode
            auto client = HttpClient::Make(item->stream_host_, item->stream_port_, "/panel/stream/message", 3000);
            auto res = client->Post({}, msg->AsJson());
            LOGI("res: {} {}", res.status, res.body);
            if (res.status == 200) {
                try {
                    auto obj = json::parse(res.body);
                    auto code = obj["code"].get<int>();
                    if (code == 200) {
                        return true;
                    }
                    else {
                        LOGE("NotifyEvent failed, error code: {}", code);
                    }
                } catch(std::exception& e) {
                    LOGE("NotifyEvent, parse json failed: {}, body: {}", e.what(), res.body);
                }
            }
            return false;
        }
    }

}

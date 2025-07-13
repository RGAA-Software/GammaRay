//
// Created by RGAA on 2024/1/17.
//

#include "gr_context.h"

#include "tc_common_new/task_runtime.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/uuid.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_common_new/log.h"
#include "tc_common_new/time_util.h"
#include "tc_3rdparty/json/json.hpp"
#include "gr_settings.h"
#include "render_panel/database/db_game_operator.h"
#include "gr_resources.h"
#include "gr_render_controller.h"
#include "gr_run_game_manager.h"
#include "gr_app_messages.h"
#include "tc_common_new/hardware.h"
#include "tc_common_new/md5.h"
#include "service/service_manager.h"
#include "gr_settings.h"
#include "gr_application.h"
#include "database/stream_db_operator.h"
#include "tc_spvr_client/spvr_api.h"
#include "devices/running_stream_manager.h"
#include "tc_qt_widget/notify/notifymanager.h"
#include "tc_dialog.h"
#include "tc_label.h"
#include "gr_workspace.h"
#include "database/gr_database.h"
#include "tc_account_sdk/acc_sdk.h"
#include "tc_relay_client/relay_api.h"
#include "relay_message.pb.h"
#include <QApplication>

using namespace nlohmann;

namespace tc
{

    GrContext::GrContext(QWidget* main_window) : QObject(nullptr) {
        main_window_ = main_window;
    }

    void GrContext::Init(const std::shared_ptr<GrApplication>& app) {
        app_ = app;
        settings_ = GrSettings::Instance();
        sp_ = SharedPreference::Instance();

        database_ = std::make_shared<GrDatabase>(shared_from_this());
        database_->Init();
        stream_db_mgr_ = database_->GetStreamDBOperator();
        db_game_manager_ = database_->GetDBGameOperator();

        auto hardware = Hardware::Instance();
        auto beg = TimeUtil::GetCurrentTimestamp();
        hardware->Detect(false, true, false);
        hardware->Dump();
        auto end = TimeUtil::GetCurrentTimestamp();
        LOGI("Detect hardware info used: {}ms", end-beg);

        srv_manager_ = std::make_shared<GrRenderController>(app);

        task_rt_ = std::make_shared<TaskRuntime>(8);

        steam_mgr_ = SteamManager::Make();
        steam_mgr_->ScanInstalledSteamPath();

        msg_notifier_ = app_->GetMessageNotifier();

        acc_sdk_ = std::make_shared<AccountSdk>(msg_notifier_, std::make_shared<AccountParams>(AccountParams {
            .host_ = "rgaa.vip",
            .port_ = 5566,
        }));

        // ips
        ips_ = IPUtil::ScanIPs();

        LOGI("Scan IP size: {}", ips_.size());
        for (auto& item : ips_) {
            LOGI("IP: {} -> {}", item.ip_addr_, item.nt_type_ == IPNetworkType::kWired ? "WIRED" : "WIRELESS");
        }

        res_manager_ = std::make_shared<GrResources>(shared_from_this());
        res_manager_->ExtractIconsIfNeeded();

        run_game_manager_ = std::make_shared<GrRunGameManager>(shared_from_this());

        service_manager_ = ServiceManager::Make();
        std::string base_path = qApp->applicationDirPath().toStdString();
        std::string bin_path = std::format("{}/GammaRayService.exe {}", base_path, settings_->sys_service_port_);
        LOGI("Service path: {}", bin_path);
        service_manager_->Init("GammaRayService", bin_path, "GammaRat Service", "** GammaRay Service **");
        service_manager_->Install();

        running_stream_mgr_ = std::make_shared<RunningStreamManager>(shared_from_this());

        notify_mgr_ = std::make_shared<NotifyManager>(main_window_);
        connect(notify_mgr_.get(), &NotifyManager::notifyDetail, this, [=, this](const NotifyItem& data) {
            this->PostTask([=, this]() {
                this->SendAppMessage(MsgNotificationClicked {
                    .data_ = data,
                });
            });
        });

        StartTimers();
    }

    void GrContext::Exit() {
        srv_manager_->Exit();
    }

    std::shared_ptr<SteamManager> GrContext::GetSteamManager() {
        return steam_mgr_;
    }

    void GrContext::PostTask(std::function<void()>&& task) {
        task_rt_->Post(SimpleThreadTask::Make(std::move(task)));
    }

    void GrContext::PostTask(std::function<std::any()>&& exec_task, std::function<void(std::any)>&& cbk_task) {
        task_rt_->Post(
            ReturnThreadTask<ExecFunc, CallbackFunc>::Make(std::move(exec_task), std::move(cbk_task))
        );
    }

    void GrContext::PostUITask(std::function<void()>&& task) {
        QMetaObject::invokeMethod(this, [=]() {
            task();
        });
    }

    void GrContext::PostUIDelayTask(std::function<void()>&& task, int ms) {
        this->PostUITask([ms, t = std::move(task)]() {
            QTimer::singleShot(ms, [=]() {
                t();
            });
        });
    }

    void GrContext::PostDBTask(std::function<void()>&& task) {
        task_rt_->GetLastThread()->Post(SimpleThreadTask::Make(std::move(task)));
    }

    void GrContext::PostDBTask(std::function<std::any()>&& exec_task, std::function<void(std::any)>&& cbk_task) {
        task_rt_->GetLastThread()->Post(
                ReturnThreadTask<ExecFunc, CallbackFunc>::Make(std::move(exec_task), std::move(cbk_task))
        );
    }

    int GrContext::GetIndexByUniqueId() {
        return std::atoi(settings_->GetDeviceId().c_str())%30+1;
    }

    std::vector<EthernetInfo> GrContext::GetIps() {
        return ips_;
    }

    std::string GrContext::GetDeviceIdOrIpAddress() {
        auto ips = this->GetIps();
        std::string ip_address;
        if (!ips.empty()) {
            ip_address = ips[0].ip_addr_;
        }
        auto device_id = settings_->GetDeviceId();
        return !device_id.empty() ? device_id : ip_address;
    }

    std::string GrContext::MakeBroadcastMessage() {
        json obj;
        // device
        obj["device_id"] = settings_->GetDeviceId();
        obj["random_pwd"] = settings_->GetDeviceRandomPwd();
        obj["icon_idx"] = this->GetIndexByUniqueId();
        // ips
        auto ip_array = json::array();
        auto ips = this->GetIps();
        for (auto& item : ips) {
            json ip_obj;
            ip_obj["ip"] = item.ip_addr_;
            ip_obj["type"] = item.nt_type_ == IPNetworkType::kWired ? "WIRED" : "WIRELESS";
            ip_array.push_back(ip_obj);
        }
        obj["ips"] = ip_array;
        obj["panel_srv_port"] = settings_->GetPanelServerPort();
        obj["render_srv_port"] = settings_->GetRenderServerPort();
        return obj.dump();
    }

    std::shared_ptr<DBGameOperator> GrContext::GetDBGameManager() {
        return db_game_manager_;
    }

    std::shared_ptr<MessageNotifier> GrContext::GetMessageNotifier() {
        return msg_notifier_;
    }

    std::shared_ptr<MessageListener> GrContext::ObtainMessageListener() {
        return msg_notifier_->CreateListener();
    }

    std::shared_ptr<GrRenderController> GrContext::GetRenderController() {
        return srv_manager_;
    }

    void GrContext::StartTimers() {
        timer_ = std::make_shared<asio2::timer>();
        timer_->start_timer(1, 100, [=, this]() {
            this->SendAppMessage(MsgGrTimer100{});
        });

        timer_->start_timer(2, 1000, [=, this]() {
            this->SendAppMessage(MsgGrTimer1S{});
        });

        timer_->start_timer(5, 5000, [=, this]() {
            this->SendAppMessage(MsgGrTimer5S{});
        });
    }

    std::shared_ptr<GrRunGameManager> GrContext::GetRunGameManager() {
        return run_game_manager_;
    }

    std::string GrContext::GetCurrentExeFolder() {
        return QCoreApplication::applicationDirPath().toStdString();
    }

    std::shared_ptr<ServiceManager> GrContext::GetServiceManager() {
        return service_manager_;
    }

    std::shared_ptr<GrApplication> GrContext::GetApplication() {
        return app_;
    }

    std::shared_ptr<StreamDBOperator> GrContext::GetStreamDBManager() {
        return stream_db_mgr_;
    }

    std::shared_ptr<RunningStreamManager> GrContext::GetRunningStreamManager() {
        return running_stream_mgr_;
    }

    std::shared_ptr<NotifyManager> GrContext::GetNotifyManager() {
        return notify_mgr_;
    }

    void GrContext::NotifyAppMessage(const QString& title, const QString& msg, std::function<void()>&& cbk) {
        QMetaObject::invokeMethod(this, [=, this]() {
            if (notify_mgr_) {
                notify_mgr_->notify(NotifyItem {
                    .type_ = NotifyItemType::kNormal,
                    .title_ = title,
                    .body_ = msg,
                    .cbk_ = cbk,
                });
            }
        });
    }

    void GrContext::NotifyAppErrMessage(const QString& title, const QString& msg, std::function<void()>&& cbk) {
        QMetaObject::invokeMethod(this, [=, this]() {
            if (notify_mgr_) {
                notify_mgr_->notify(NotifyItem {
                    .type_ = NotifyItemType::kError,
                    .title_ = title,
                    .body_ = msg,
                    .cbk_ = cbk,
                });
            }
        });
    }

    std::shared_ptr<GrDatabase> GrContext::GetDatabase() {
        return database_;
    }

    std::shared_ptr<AccountSdk> GrContext::GetAccSdk() {
        return acc_sdk_;
    }

    std::shared_ptr<relay::RelayDeviceInfo> GrContext::GetRelayServerSideDeviceInfo(const std::string& relay_host,
                                                                                    int relay_port,
                                                                                    const std::string& device_id,
                                                                                    bool show_dialog) {
        if (!settings_->HasRelayServerConfig()) {
            return nullptr;
        }
        auto srv_remote_device_id = "server_" + device_id;
        auto relay_result = relay::RelayApi::GetRelayDeviceInfo(relay_host, relay_port, srv_remote_device_id);
        if (!relay_result) {
            LOGE("Get device info for: {} failed: {}", srv_remote_device_id, relay::RelayError2String(relay_result.error()));
            if (show_dialog) {
                TcDialog dialog(tcTr("id_error"), tcTr("id_cant_get_remote_device_info"), grWorkspace.get());
                dialog.exec();
            }
            return nullptr;
        }
        auto relay_device_info = relay_result.value();
        //LOGI("Remote device info: id: {}, relay host: {}, port: {}",
        //     srv_remote_device_id, relay_device_info->relay_server_ip_, relay_device_info->relay_server_port_);
        return relay_device_info;

    }

}
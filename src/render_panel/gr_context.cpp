//
// Created by RGAA on 2024/1/17.
//

#include "gr_context.h"

#include "tc_common_new/task_runtime.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/uuid.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_common_new/log.h"
#include "tc_3rdparty/json/json.hpp"
#include "gr_settings.h"
#include "render_panel/db/db_game_manager.h"
#include "gr_resources.h"
#include "gr_render_controller.h"
#include "gr_run_game_manager.h"
#include "gr_app_messages.h"
#include "tc_common_new/hardware.h"
#include "tc_common_new/md5.h"
#include "service/service_manager.h"
#include "gr_settings.h"
#include "gr_application.h"
#include "devices/stream_db_manager.h"
#include "tc_spvr_client/spvr_manager.h"
#include "devices/running_stream_manager.h"
#include <QApplication>

using namespace nlohmann;

namespace tc
{

    GrContext::GrContext() : QObject(nullptr) {

    }

    void GrContext::Init(const std::shared_ptr<GrApplication>& app) {
        app_ = app;
        settings_ = GrSettings::Instance();
        sp_ = SharedPreference::Instance();

        auto hardware = Hardware::Instance();
        hardware->Detect(false, true, false);
        hardware->Dump();

        srv_manager_ = std::make_shared<GrRenderController>(app);

        task_rt_ = std::make_shared<TaskRuntime>(8);

        steam_mgr_ = SteamManager::Make();
        steam_mgr_->ScanInstalledSteamPath();

        msg_notifier_ = app_->GetMessageNotifier();

        stream_db_mgr_ = std::make_shared<StreamDBManager>();

        // ips
        ips_ = IPUtil::ScanIPs();

        LOGI("Scan IP size: {}", ips_.size());
        for (auto& item : ips_) {
            LOGI("IP: {} -> {}", item.ip_addr_, item.nt_type_ == IPNetworkType::kWired ? "WIRED" : "WIRELESS");
        }

        db_game_manager_ = std::make_shared<DBGameManager>(shared_from_this());
        db_game_manager_->Init();

        res_manager_ = std::make_shared<GrResources>(shared_from_this());
        res_manager_->ExtractIconsIfNeeded();

        run_game_manager_ = std::make_shared<GrRunGameManager>(shared_from_this());

        service_manager_ = ServiceManager::Make();
        std::string base_path = qApp->applicationDirPath().toStdString();
        std::string bin_path = std::format("{}/GammaRayService.exe {}", base_path, settings_->sys_service_port_);
        LOGI("Service path: {}", bin_path);
        service_manager_->Init("GammaRayService", bin_path, "GammaRat Service", "** GammaRay Service **");
        service_manager_->Install();

        spvr_mgr_ = std::make_shared<SpvrManager>();
        spvr_mgr_->SetHostPort(settings_->spvr_server_host_, std::atoi(settings_->spvr_server_port_.c_str()));

        running_stream_mgr_ = std::make_shared<RunningStreamManager>(shared_from_this());

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

    int GrContext::GetIndexByUniqueId() {
        return std::atoi(settings_->device_id_.c_str())%30+1;
    }

    std::vector<EthernetInfo> GrContext::GetIps() {
        return ips_;
    }

    std::string GrContext::MakeBroadcastMessage() {
        json obj;
        // sys id
        obj["sys_unique_id"] = settings_->device_id_;
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
        // ports
        obj["http_server_port"] = settings_->http_server_port_;
        obj["ws_server_port"] = settings_->panel_listen_port_;
        obj["udp_listen_port"] = settings_->udp_listen_port_;
        obj["stream_ws_port"] = settings_->network_listening_port_;

        return obj.dump();
    }

    std::shared_ptr<DBGameManager> GrContext::GetDBGameManager() {
        return db_game_manager_;
    }

    std::shared_ptr<MessageNotifier> GrContext::GetMessageNotifier() {
        return msg_notifier_;
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

    std::shared_ptr<StreamDBManager> GrContext::GetStreamDBManager() {
        return stream_db_mgr_;
    }

    std::shared_ptr<SpvrManager> GrContext::GetSpvrManager() {
        return spvr_mgr_;
    }

    std::shared_ptr<RunningStreamManager> GrContext::GetRunningStreamManager() {
        return running_stream_mgr_;
    }

}
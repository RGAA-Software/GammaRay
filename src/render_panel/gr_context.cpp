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
#include <QApplication>

using namespace nlohmann;

namespace tc
{

    constexpr auto kKeySysUniqueId = "sys_unique_id";

    GrContext::GrContext() : QObject(nullptr) {

    }

    void GrContext::Init(const std::shared_ptr<GrApplication>& app) {
        app_ = app;
        settings_ = GrSettings::Instance();
        sp_ = SharedPreference::Instance();

        auto hardware = Hardware::Instance();
        hardware->Detect(false, true, false);
        hardware->Dump();

        // unique id
        LoadUniqueId();

        srv_manager_ = std::make_shared<GrRenderController>(app);

        task_runtime_ = std::make_shared<TaskRuntime>();
        steam_mgr_ = SteamManager::Make(task_runtime_);
        steam_mgr_->ScanInstalledSteamPath();

        msg_notifier_ = std::make_shared<MessageNotifier>();

        // ips
        ips_ = IPUtil::ScanIPs();

        LOGI("Unique Id: {}", unique_id_);
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

        StartTimers();
    }

    void GrContext::Exit() {
        srv_manager_->Exit();
    }

    std::shared_ptr<SteamManager> GrContext::GetSteamManager() {
        return steam_mgr_;
    }

    std::shared_ptr<TaskRuntime> GrContext::GetTaskRuntime() {
        return task_runtime_;
    }

    void GrContext::PostTask(std::function<void()>&& task) {
        task_runtime_->Post(SimpleThreadTask::Make(std::move(task)));
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

    void GrContext::LoadUniqueId() {
        unique_id_ = sp_->Get(kKeySysUniqueId);
        if (unique_id_.empty()) {
            GenUniqueId();
            sp_->Put(kKeySysUniqueId, unique_id_);
        }
    }

    void GrContext::GenUniqueId() {
        auto hardware = Hardware::Instance();
        auto disks = hardware->hw_disks_;
        std::string seed;
        if (!disks.empty()) {
            for (const auto& disk : disks) {
                seed = seed.append(disk.serial_number_);
            }
        } else {
            seed = GetUUID();
        }
        LOGI("Seed: {}, disks size: {}", seed, disks.size());
        auto md5_str = MD5::Hex(seed);
        std::stringstream ss;
        ss
        << md5_str[0]%10
        << md5_str[7]%10
        << md5_str[11]%10
        << md5_str[16]%10
        << md5_str[18]%10
        << md5_str[23]%10
        << md5_str[26]%10
        << md5_str[28]%10
        << md5_str[30]%10;
        unique_id_ = ss.str();
    }

    std::string GrContext::GetSysUniqueId() {
        return unique_id_;
    }

    int GrContext::GetIndexByUniqueId() {
        return std::atoi(GetSysUniqueId().c_str())%30+1;
    }

    std::vector<EthernetInfo> GrContext::GetIps() {
        return ips_;
    }

    std::string GrContext::MakeBroadcastMessage() {
        json obj;
        // sys id
        obj["sys_unique_id"] = this->GetSysUniqueId();
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
        obj["ws_server_port"] = settings_->ws_server_port_;
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

}
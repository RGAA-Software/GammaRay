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
#include "db/db_game_manager.h"
#include "gr_resources.h"
#include "gr_server_manager.h"
#include "gr_run_game_manager.h"
#include "app_messages.h"
#include <QApplication>

using namespace nlohmann;

namespace tc
{

    constexpr auto kKeySysUniqueId = "sys_unique_id";

    GrContext::GrContext() : QObject(nullptr) {

    }

    void GrContext::Init() {
        settings_ = GrSettings::Instance();
        sp_ = SharedPreference::Instance();
        // unique id
        LoadUniqueId();

        srv_manager_ = std::make_shared<GrServerManager>(shared_from_this());

        task_runtime_ = std::make_shared<TaskRuntime>();
        steam_mgr_ = SteamManager::Make(task_runtime_);
        steam_mgr_->ScanInstalledSteamPath();

        msg_notifier_ = std::make_shared<MessageNotifier>();

        // ips
        ips_ = IPUtil::ScanIPs();

        LOGI("Unique Id: {}", unique_id_);
        LOGI("Scan IP size: {}", ips_.size());
        for (auto& [ip, type] : ips_) {
            LOGI("IP: {} -> {}", ip, type == IPNetworkType::kWired ? "WIRED" : "WIRELESS");
        }

        db_game_manager_ = std::make_shared<DBGameManager>(shared_from_this());
        db_game_manager_->Init();

        res_manager_ = std::make_shared<GrResources>(shared_from_this());
        res_manager_->ExtractIconsIfNeeded();

        run_game_manager_ = std::make_shared<GrRunGameManager>(shared_from_this());

        StartTimers();
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
        auto uuid = GetUUID();
        std::hash<std::string> fn_hash;
        size_t value = fn_hash(uuid);
        unique_id_ = std::to_string(value%1000000);
    }

    std::string GrContext::GetSysUniqueId() {
        return unique_id_;
    }

    int GrContext::GetIndexByUniqueId() {
        return std::atoi(GetSysUniqueId().c_str())%30+1;
    }

    std::map<std::string, IPNetworkType> GrContext::GetIps() {
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
        for (auto& [ip, type] : ips) {
            json ip_obj;
            ip_obj["ip"] = ip;
            ip_obj["type"] = type == IPNetworkType::kWired ? "WIRED" : "WIRELESS";
            ip_array.push_back(ip_obj);
        }
        obj["ips"] = ip_array;
        // ports
        obj["http_server_port"] = settings_->http_server_port_;
        obj["ws_server_port"] = settings_->ws_server_port_;
        obj["udp_server_port"] = settings_->udp_server_port_;
        obj["stream_ws_port"] = settings_->network_listen_port_;

        return obj.dump();
    }

    std::shared_ptr<DBGameManager> GrContext::GetDBGameManager() {
        return db_game_manager_;
    }

    std::shared_ptr<MessageNotifier> GrContext::GetMessageNotifier() {
        return msg_notifier_;
    }

    std::shared_ptr<GrServerManager> GrContext::GetServerManager() {
        return srv_manager_;
    }

    void GrContext::StartTimers() {
        timer_ = std::make_shared<asio2::timer>();
        timer_->start_timer(1, 100, [=, this]() {
            this->SendAppMessage(MsgGrTimer100{});
        });
    }

    std::shared_ptr<GrRunGameManager> GrContext::GetRunGameManager() {
        return run_game_manager_;
    }

    std::string GrContext::GetCurrentExeFolder() {
        return QCoreApplication::applicationDirPath().toStdString();
    }

}
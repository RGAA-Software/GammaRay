//
// Created by hy on 2024/1/17.
//

#include "context.h"

#include "tc_common_new/task_runtime.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/uuid.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_common_new/log.h"
#include "tc_3rdparty/json/json.hpp"
#include "settings.h"
#include "db/game_manager.h"
#include "res/resource_manager.h"

using namespace nlohmann;

namespace tc
{

    constexpr auto kKeySysUniqueId = "sys_unique_id";

    Context::Context() : QObject(nullptr) {

    }

    void Context::Init() {
        settings_ = Settings::Instance();
        sp_ = SharedPreference::Instance();
        sp_->Init("", "tc_steam.dat");
        // unique id
        LoadUniqueId();

        task_runtime_ = std::make_shared<TaskRuntime>();
        steam_mgr_ = SteamManager::Make(task_runtime_);

        // ips
        ips_ = IPUtil::ScanIPs();

        LOGI("Unique Id: {}", unique_id_);
        LOGI("Scan IP size: {}", ips_.size());
        for (auto& [ip, type] : ips_) {
            LOGI("IP: {} -> {}", ip, type == IPNetworkType::kWired ? "Wired" : "Wireless");
        }

        game_manager_ = std::make_shared<GameManager>(shared_from_this());
        game_manager_->Init();

        res_manager_ = std::make_shared<ResourceManager>(shared_from_this());
        res_manager_->ExtractIconsIfNeeded();
    }

    std::shared_ptr<SteamManager> Context::GetSteamManager() {
        return steam_mgr_;
    }

    std::shared_ptr<TaskRuntime> Context::GetTaskRuntime() {
        return task_runtime_;
    }

    void Context::PostTask(std::function<void()>&& task) {
        task_runtime_->Post(SimpleThreadTask::Make(std::move(task)));
    }

    void Context::PostUITask(std::function<void()>&& task) {
        QMetaObject::invokeMethod(this, [=]() {
            task();
        });
    }

    void Context::LoadUniqueId() {
        unique_id_ = sp_->Get(kKeySysUniqueId);
        if (unique_id_.empty()) {
            GenUniqueId();
            sp_->Put(kKeySysUniqueId, unique_id_);
        }
    }

    void Context::GenUniqueId() {
        auto uuid = GetUUID();
        std::hash<std::string> fn_hash;
        size_t value = fn_hash(uuid);
        unique_id_ = std::to_string(value%1000000);
    }

    std::string Context::GetSysUniqueId() {
        return unique_id_;
    }

    std::map<std::string, IPNetworkType> Context::GetIps() {
        return ips_;
    }

    std::string Context::MakeBroadcastMessage() {
        json obj;
        // sys id
        obj["sys_unique_id"] = this->GetSysUniqueId();
        // ips
        auto ip_array = json::array();
        auto ips = this->GetIps();
        for (auto& [ip, type] : ips) {
            json ip_obj;
            ip_obj["ip"] = ip;
            ip_obj["type"] = type == IPNetworkType::kWired ? "Wired" : "Wireless";
            ip_array.push_back(ip_obj);
        }
        obj["ips"] = ip_array;
        // ports
        obj["http_server_port"] = settings_->http_server_port_;
        obj["ws_server_port"] = settings_->ws_server_port_;
        obj["udp_server_port"] = settings_->udp_server_port_;

        return obj.dump();
    }

    std::shared_ptr<GameManager> Context::GetGameManager() {
        return game_manager_;
    }

}
//
// Created by RGAA on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_CONTEXT_H
#define TC_SERVER_STEAM_CONTEXT_H

#include <memory>
#include <string>
#include "network/ip_util.h"
#include "tc_common_new/message_notifier.h"
#include <asio2/asio2.hpp>

#include <QObject>
#include <QTimer>

namespace tc
{

    class SteamManager;
    class TaskRuntime;
    class SharedPreference;
    class GrSettings;
    class DBGameManager;
    class ResourceManager;
    class ServerManager;
    class RunGameManager;

    class GrContext : public QObject, public std::enable_shared_from_this<GrContext> {
    public:
        GrContext();

        void Init();
        std::shared_ptr<SteamManager> GetSteamManager();
        std::shared_ptr<TaskRuntime> GetTaskRuntime();
        void PostTask(std::function<void()>&& task);
        void PostUITask(std::function<void()>&& task);
        void PostDelayTask(std::function<void()>&& task, int ms);

        std::string GetSysUniqueId();
        int GetIndexByUniqueId();
        std::map<std::string, IPNetworkType> GetIps();

        std::string MakeBroadcastMessage();

        std::shared_ptr<DBGameManager> GetDBGameManager();

        template<typename T>
        void SendAppMessage(const T& m) {
            if (msg_notifier_) {
                msg_notifier_->SendAppMessage(m);
            }
        }
        std::shared_ptr<MessageNotifier> GetMessageNotifier();
        std::shared_ptr<ServerManager> GetServerManager();
        std::shared_ptr<RunGameManager> GetRunGameManager();

    private:
        void LoadUniqueId();
        void GenUniqueId();
        void StartTimers();

    private:
        GrSettings* settings_ = nullptr;
        SharedPreference* sp_ = nullptr;
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;
        std::shared_ptr<TaskRuntime> task_runtime_ = nullptr;
        std::string unique_id_{};
        std::map<std::string, IPNetworkType> ips_;
        std::shared_ptr<DBGameManager> db_game_manager_ = nullptr;
        std::shared_ptr<ResourceManager> res_manager_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::shared_ptr<ServerManager> srv_manager_ = nullptr;
        std::shared_ptr<asio2::timer> timer_ = nullptr;
        std::shared_ptr<RunGameManager> run_game_manager_ = nullptr;
    };

}
#endif //TC_SERVER_STEAM_CONTEXT_H

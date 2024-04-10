//
// Created by hy on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_CONTEXT_H
#define TC_SERVER_STEAM_CONTEXT_H

#include <memory>
#include <string>
#include "network/ip_util.h"

#include <QObject>

namespace tc
{

    class SteamManager;
    class TaskRuntime;
    class SharedPreference;
    class Settings;

    class Context : public QObject, public std::enable_shared_from_this<Context> {
    public:
        Context();

        void Init();
        std::shared_ptr<SteamManager> GetSteamManager();
        std::shared_ptr<TaskRuntime> GetTaskRuntime();
        void PostTask(std::function<void()>&& task);
        void PostUITask(std::function<void()>&& task);

        std::string GetSysUniqueId();
        std::map<std::string, IPNetworkType> GetIps();

        std::string MakeBroadcastMessage();

    private:
        void LoadUniqueId();
        void GenUniqueId();

    private:
        Settings* settings_ = nullptr;
        SharedPreference* sp_ = nullptr;
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;
        std::shared_ptr<TaskRuntime> task_runtime_ = nullptr;
        std::string unique_id_{};
        std::map<std::string, IPNetworkType> ips_;
    };

}
#endif //TC_SERVER_STEAM_CONTEXT_H

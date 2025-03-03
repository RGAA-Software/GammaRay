//
// Created by RGAA on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_CONTEXT_H
#define TC_SERVER_STEAM_CONTEXT_H

#include <memory>
#include <string>
#include "tc_common_new/ip_util.h"
#include "tc_common_new/message_notifier.h"
#include <asio2/asio2.hpp>

#include <QObject>
#include <QTimer>

namespace tc
{

    class SteamManager;
    class SharedPreference;
    class GrSettings;
    class DBGameManager;
    class GrResources;
    class GrRenderController;
    class GrRunGameManager;
    class ServiceManager;
    class GrApplication;

    class GrContext : public QObject, public std::enable_shared_from_this<GrContext> {
    public:
        GrContext();

        void Init(const std::shared_ptr<GrApplication>& app);
        void Exit();
        std::shared_ptr<SteamManager> GetSteamManager();
        void PostTask(std::function<void()>&& task);
        void PostUITask(std::function<void()>&& task);
        void PostUIDelayTask(std::function<void()>&& task, int ms);

        std::string GetSysUniqueId();
        int GetIndexByUniqueId();
        std::vector<EthernetInfo> GetIps();

        std::string MakeBroadcastMessage();

        std::shared_ptr<DBGameManager> GetDBGameManager();
        std::shared_ptr<ServiceManager> GetServiceManager();

        template<typename T>
        void SendAppMessage(const T& m) {
            if (msg_notifier_) {
                msg_notifier_->SendAppMessage(m);
            }
        }
        std::shared_ptr<MessageNotifier> GetMessageNotifier();
        std::shared_ptr<GrRenderController> GetRenderController();
        std::shared_ptr<GrRunGameManager> GetRunGameManager();
        static std::string GetCurrentExeFolder();

    private:
        void LoadUniqueId();
        void GenUniqueId();
        void StartTimers();

    private:
        GrSettings* settings_ = nullptr;
        SharedPreference* sp_ = nullptr;
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;
        std::shared_ptr<asio2::iopool> asio2_pool_ = nullptr;
        std::string unique_id_{};
        std::vector<EthernetInfo> ips_;
        std::shared_ptr<DBGameManager> db_game_manager_ = nullptr;
        std::shared_ptr<GrResources> res_manager_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::shared_ptr<GrRenderController> srv_manager_ = nullptr;
        std::shared_ptr<asio2::timer> timer_ = nullptr;
        std::shared_ptr<GrRunGameManager> run_game_manager_ = nullptr;
        std::shared_ptr<ServiceManager> service_manager_ =  nullptr;
    };

}
#endif //TC_SERVER_STEAM_CONTEXT_H

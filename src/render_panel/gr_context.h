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
#include "expt/expected.h"

#include <QObject>
#include <QTimer>

namespace relay
{
    class RelayDeviceInfo;
}

namespace tc
{

    class SteamManager;
    class SharedPreference;
    class GrSettings;
    class DBGameOperator;
    class GrResources;
    class GrRenderController;
    class GrRunGameManager;
    class ServiceManager;
    class GrApplication;
    class NotifyManager;
    class GrDatabase;
    class AccountSdk;

    // Device list
    class StreamDBOperator;
    class TaskRuntime;
    class SpvrManager;
    class RunningStreamManager;

    class GrContext : public QObject, public std::enable_shared_from_this<GrContext> {
    public:
        explicit GrContext(QWidget* main_window);

        void Init(const std::shared_ptr<GrApplication>& app);
        void Exit();
        std::shared_ptr<SteamManager> GetSteamManager();
        // Post task in runtime
        void PostTask(std::function<void()>&& task);

        // Post task in runtime and receive returned value
        // like:
        // auto ret = exec_task();
        // ckb_task(ret);
        void PostTask(std::function<std::any()>&& exec_task, std::function<void(std::any)>&& cbk_task);

        // Post in Qt UI Thread(Windows messages looping thread)
        void PostUITask(std::function<void()>&& task);

        // Post in Qt UI Thread(Windows messages looping thread), task will exec after specific milliseconds
        void PostUIDelayTask(std::function<void()>&& task, int ms);

        // Like PostTask, but always in a fixed thread
        void PostDBTask(std::function<void()>&& task);

        // Like PostTask, but always in a fixed thread
        void PostDBTask(std::function<std::any()>&& exec_task, std::function<void(std::any)>&& cbk_task);

        int GetIndexByUniqueId();
        std::vector<EthernetInfo> GetIps();

        std::string MakeBroadcastMessage();

        std::shared_ptr<DBGameOperator> GetDBGameManager();
        std::shared_ptr<ServiceManager> GetServiceManager();

        template<typename T>
        void SendAppMessage(const T& m) {
            if (msg_notifier_) {
                msg_notifier_->SendAppMessage(m);
            }
        }
        std::shared_ptr<MessageNotifier> GetMessageNotifier();
        std::shared_ptr<MessageListener> ObtainMessageListener();
        std::shared_ptr<GrRenderController> GetRenderController();
        std::shared_ptr<GrRunGameManager> GetRunGameManager();
        static std::string GetCurrentExeFolder();
        std::shared_ptr<StreamDBOperator> GetStreamDBManager();
        std::shared_ptr<RunningStreamManager> GetRunningStreamManager();
        std::shared_ptr<GrDatabase> GetDatabase();
        std::shared_ptr<AccountSdk> GetAccSdk();

        // Display a message on right-bottom
        std::shared_ptr<NotifyManager> GetNotifyManager();
        void NotifyAppMessage(const QString& title, const QString& msg);
        void NotifyAppErrMessage(const QString& title, const QString& msg);

        // spvr
        // will add prefix: server
        // id ==> server_111333444
        std::shared_ptr<relay::RelayDeviceInfo> GetRelayServerSideDeviceInfo(const std::string& device_id, bool show_dialog = true);

    private:
        void StartTimers();

    private:
        QWidget* main_window_ = nullptr;
        GrSettings* settings_ = nullptr;
        SharedPreference* sp_ = nullptr;
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;
        std::shared_ptr<TaskRuntime> task_rt_ = nullptr;
        std::vector<EthernetInfo> ips_;
        std::shared_ptr<DBGameOperator> db_game_manager_ = nullptr;
        std::shared_ptr<GrResources> res_manager_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::shared_ptr<GrRenderController> srv_manager_ = nullptr;
        std::shared_ptr<asio2::timer> timer_ = nullptr;
        std::shared_ptr<GrRunGameManager> run_game_manager_ = nullptr;
        std::shared_ptr<ServiceManager> service_manager_ =  nullptr;
        std::shared_ptr<StreamDBOperator> stream_db_mgr_ = nullptr;
        std::shared_ptr<SpvrManager> spvr_mgr_ = nullptr;
        std::shared_ptr<RunningStreamManager> running_stream_mgr_ = nullptr;
        std::shared_ptr<NotifyManager> notify_mgr_ = nullptr;
        std::shared_ptr<GrDatabase> database_ = nullptr;
        std::shared_ptr<AccountSdk> acc_sdk_ = nullptr;
    };

}
#endif //TC_SERVER_STEAM_CONTEXT_H

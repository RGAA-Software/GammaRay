//
// Created by RGAA on 2023-12-27.
//

#ifndef TC_CLIENT_PC_CLIENT_CONTEXT_H
#define TC_CLIENT_PC_CLIENT_CONTEXT_H

#include <functional>

#include <QObject>
#include <QWidget>
#include <map>
#include "tc_common_new/message_notifier.h"
#include "tc_client_sdk_new/sdk_messages.h"

namespace tc
{
    class AppMessage;
    class Thread;
    class StreamDBOperator;
    class SharedPreference;
    class ClientPluginManager;
    class NotifyManager;

    class ClientContext : public QObject, public std::enable_shared_from_this<ClientContext> {
    public:
        explicit ClientContext(const std::string& name, QObject* parent = nullptr);
        ~ClientContext() override;
        void Init();
        void PostTask(std::function<void()>&& task);
        void PostUITask(std::function<void()>&& task);
        void PostDelayUITask(std::function<void()>&& task, int ms);
        std::shared_ptr<MessageNotifier> GetMessageNotifier();
        std::shared_ptr<MessageListener> ObtainMessageListener();
        void SaveKeyValue(const std::string& k, const std::string& v);
        std::string GetValueByKey(const std::string& k);
        void UpdateCapturingMonitorInfo(const SdkCaptureMonitorInfo& info);
        std::map<std::string, SdkCaptureMonitorInfo> GetCapturingMonitorInfoMap();

        void SetPluginManager(const std::shared_ptr<ClientPluginManager>& mgr);
        std::shared_ptr<ClientPluginManager> GetPluginManager();

        template<class T>
        void SendAppMessage(const T& msg) {
            msg_notifier_->SendAppMessage(msg);
        }

        void Exit();

        void SetRecording(bool recording);
        bool GetRecording();

        void InitNotifyManager(QWidget* parent);
        std::shared_ptr<NotifyManager> GetNotifyManager() const;
        void NotifyAppMessage(const QString& title, const QString& msg, std::function<void()>&& cbk = []() {});
        void NotifyAppErrMessage(const QString& title, const QString& msg, std::function<void()>&& cbk = []() {});

        bool full_functionality_ = false;
    private:
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        SharedPreference* sp_ = nullptr;
        std::shared_ptr<Thread> task_thread_ = nullptr;
        std::string name_;
        std::map<std::string, SdkCaptureMonitorInfo> capturing_info_map_;

        // plugin manager
        std::shared_ptr<ClientPluginManager> plugin_mgr_ = nullptr;

        //是否在录制中
        std::atomic_bool recording_ = false;

        std::shared_ptr<NotifyManager> notify_manager_ = nullptr;
    };

}

#endif //TC_CLIENT_PC_CLIENT_CONTEXT_H

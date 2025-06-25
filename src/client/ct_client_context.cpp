//
// Created by RGAA on 2023-12-27.
//

#include "client/ct_client_context.h"

#include "client/ct_settings.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/hardware.h"
#include "tc_qt_widget/notify/notifymanager.h"
#include "client/ct_app_message.h"
#include <QTimer>
#include <QApplication>

namespace tc
{

    static std::string kClientEmbedName = "ui.embed";

    ClientContext::ClientContext(const std::string& name, QObject* parent) : QObject(parent) {
        this->name_ = name;
        this->msg_notifier_ = std::make_shared<MessageNotifier>();
        this->capturing_info_ = SdkCaptureMonitorInfo {

        };
    }

    ClientContext::~ClientContext() {
        Exit();
    }

    void ClientContext::Init(bool render) {
        render_ = render;

        sp_ = SharedPreference::Instance();
        auto sp_name = std::format("./gr_data/app.{}.dat", this->name_);
        if (!sp_->Init("", sp_name)) {
            LOGE("Init sp failed: {}", sp_name);
        }

        auto base_dir = QApplication::applicationDirPath();
        auto log_path = base_dir + std::format("/gr_logs/app.{}.log", this->name_).c_str();
        std::cout << "log path: " << log_path.toStdString() << std::endl;

        if (this->name_ == kClientEmbedName) {
            // embed in main panel
            // log to gammaray.log
            // will set device id by SetDeviceId
        }
        else {
            // single running
            Logger::InitLog(log_path.toStdString(), true);
        }
        LOGI("ClientContext in {}", this->name_);

        auto settings = Settings::Instance();
        if (!render) {
            settings->LoadMainSettings();
        } else {
            settings->LoadRenderSettings();
        }

        task_thread_ = Thread::Make("context_thread", 128);
        task_thread_->Poll();

        LOGI("Client params for: {}", this->name_);
        settings->Dump();

        PostTask([=, this]() {
            auto hardware = Hardware::Instance();
            hardware->Detect(false, true, false);
            hardware->Dump();
        });
    }

    void ClientContext::PostTask(std::function<void()>&& task) {
        task_thread_->Post(SimpleThreadTask::Make(std::move(task)));
    }

    void ClientContext::PostUITask(std::function<void()>&& task) {
        QMetaObject::invokeMethod(this, [=, this]() {
            task();
        });
    }

    void ClientContext::PostDelayUITask(std::function<void()>&& task, int ms) {
        this->PostUITask([ms, t = std::move(task)]() {
            QTimer::singleShot(ms, [=]() {
                t();
            });
        });
    }

    std::shared_ptr<MessageNotifier> ClientContext::GetMessageNotifier() {
        return msg_notifier_;
    }

    std::shared_ptr<MessageListener> ClientContext::ObtainMessageListener() {
        return msg_notifier_->CreateListener();
    }

    void ClientContext::Exit() {
        if (task_thread_ && task_thread_->IsJoinable()) {
            task_thread_->Exit();
        }
    }

    void ClientContext::SaveKeyValue(const std::string& k, const std::string& v) {
        sp_->Put(k, v);
    }

    std::string ClientContext::GetValueByKey(const std::string& k) {
        return sp_->Get(k);
    }

    void ClientContext::UpdateCapturingMonitorInfo(const SdkCaptureMonitorInfo& info) {
        bool new_monitor = info.mon_name_ != capturing_info_.mon_name_;
        if (new_monitor) {
            SendAppMessage(MsgClientMonitorChanged{});
        }
        else {
            bool size_changed = capturing_info_.frame_width_ != info.frame_width_ || capturing_info_.frame_height_ != info.frame_height_;
            if (size_changed) {
                SendAppMessage(MsgClientMonitorChanged{});
            }
        }
        capturing_info_ = info;
    }

    SdkCaptureMonitorInfo ClientContext::GetCapturingMonitorInfo() {
        return capturing_info_;
    }

    std::string ClientContext::GetCapturingMonitorName() const {
        return capturing_info_.mon_name_;
    }

    void ClientContext::SetPluginManager(const std::shared_ptr<ClientPluginManager>& mgr) {
        plugin_mgr_ = mgr;
    }

    void ClientContext::SetRecording(bool recording) {
        recording_ = recording;
    }

    bool ClientContext::GetRecording() {
        return recording_;
    }

    void ClientContext::InitNotifyManager(QWidget* parent) {
        notify_manager_ = std::make_shared<NotifyManager>(parent);
        connect(notify_manager_.get(), &NotifyManager::notifyDetail, this, [=, this](const NotifyItem& data) {
            this->PostTask([=, this]() {
                this->SendAppMessage(MsgClientNotificationClicked {
                    .data_ = data,
                });
            });
        });
    }

    std::shared_ptr<NotifyManager> ClientContext::GetNotifyManager() const {
        return notify_manager_;
    }

    void ClientContext::NotifyAppMessage(const QString& title, const QString& msg, std::function<void()>&& cbk) {
        QMetaObject::invokeMethod(this, [=, this]() {
            if (notify_manager_) {
                notify_manager_->notify(NotifyItem {
                    .type_ = NotifyItemType::kNormal,
                    .title_ = title,
                    .body_ = msg,
                    .cbk_ = cbk,
                });
            }
        });
    }

    void ClientContext::NotifyAppErrMessage(const QString& title, const QString& msg, std::function<void()>&& cbk) {
        QMetaObject::invokeMethod(this, [=, this]() {
            if (notify_manager_) {
                notify_manager_->notify(NotifyItem {
                    .type_ = NotifyItemType::kError,
                    .title_ = title,
                    .body_ = msg,
                    .cbk_ = cbk,
                });
            }
        });
    }
}
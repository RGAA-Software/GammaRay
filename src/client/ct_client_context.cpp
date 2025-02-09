//
// Created by RGAA on 2023-12-27.
//

#include "client/ct_client_context.h"

#include "client/ct_settings.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "db/stream_db_manager.h"
#include "client/ct_app_message.h"
#include <QTimer>
#include <QApplication>

namespace tc
{

    ClientContext::ClientContext(const std::string& name, QObject* parent) : QObject(parent) {
        this->name_ = name;
        this->msg_notifier_ = std::make_shared<MessageNotifier>();
        this->capturing_info_ = CaptureMonitorInfo {
            .mon_idx_ = -1,
        };
    }

    ClientContext::~ClientContext() {
        Exit();
    }

    void ClientContext::Init(bool render) {
        render_ = render;
        sp_ = std::make_shared<SharedPreference>();
        sp_->Init("", std::format("./gr_data/app.{}.dat", this->name_));

        auto base_dir = QApplication::applicationDirPath();
        auto log_path = base_dir + std::format("/gr_logs/app.{}.log", this->name_).c_str();
        std::cout << "log path: " << log_path.toStdString() << std::endl;
        Logger::InitLog(log_path.toStdString(), true);

        auto settings = Settings::Instance();
        settings->SetSharedPreference(sp_);
        if (!render) {
            settings->LoadMainSettings();
        } else {
            settings->LoadRenderSettings();
        }

        db_mgr_ = std::make_shared<StreamDBManager>();
        auto stream_id = "steam_my_self";
        if (!db_mgr_->HasStream(stream_id)) {
            auto item = StreamItem {
                .stream_id = stream_id,
                .stream_name = "MY SELF",
                .stream_host = "127.0.0.1",
                .stream_port = 20371,
                .bg_color = db_mgr_->RandomColor(),
            };
            db_mgr_->AddStream(item);
        }

        task_thread_ = Thread::Make("context_thread", 128);
        task_thread_->Poll();
    }

    void ClientContext::PostTask(std::function<void()>&& task) {
        task_thread_->Post(SimpleThreadTask::Make(std::move(task)));
    }

    void ClientContext::PostUITask(std::function<void()>&& task) {
        QMetaObject::invokeMethod(this, [=, this]() {
            task();
        });
    }

    std::shared_ptr<MessageNotifier> ClientContext::GetMessageNotifier() {
        return msg_notifier_;
    }

    std::shared_ptr<MessageListener> ClientContext::ObtainMessageListener() {
        return msg_notifier_->CreateListener();
    }

    std::shared_ptr<StreamDBManager> ClientContext::GetDBManager() {
        return db_mgr_;
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

    void ClientContext::UpdateCapturingMonitorInfo(const CaptureMonitorInfo& info) {
        bool new_monitor = info.mon_idx_ != capturing_info_.mon_idx_;
        capturing_info_ = info;
        if (new_monitor) {
            SendAppMessage(MsgMonitorChanged{});
        }
    }

    CaptureMonitorInfo ClientContext::GetCapturingMonitorInfo() {
        return capturing_info_;
    }

    int ClientContext::GetCapturingMonitorIndex() const {
        return capturing_info_.mon_idx_;
    }

}
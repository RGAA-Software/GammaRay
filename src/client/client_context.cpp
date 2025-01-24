//
// Created by RGAA on 2023-12-27.
//

#include "client_context.h"

#include "settings.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/thread.h"
#include "db/stream_db_manager.h"
#include "app_message.h"
#include <QTimer>

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
        sp_->Init("", std::format("app.{}.dat", this->name_));

        auto settings = Settings::Instance();
        settings->SetSharedPreference(sp_);
        if (!render) {
            settings->LoadMainSettings();
        } else {
            settings->LoadRenderSettings();
        }

        db_mgr_ = std::make_shared<StreamDBManager>();

        task_thread_ = Thread::Make("context_thread", 128);
        task_thread_->Poll();

        io_ctx_thread_ = std::make_shared<Thread>([=, this]() {
            boost_io_ctx_ = std::make_shared<boost::asio::io_context>();
            work_guard_ = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(*boost_io_ctx_));
            boost_io_ctx_->run();
        }, "", false);
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

    std::shared_ptr<boost::asio::io_context> ClientContext::GetBoostIoContext() {
        return boost_io_ctx_;
    }

    void ClientContext::Exit() {
        if (task_thread_ && task_thread_->IsJoinable()) {
            task_thread_->Exit();
        }
        work_guard_->reset();
        boost_io_ctx_->stop();
        if (io_ctx_thread_->IsJoinable()) {
            io_ctx_thread_->Join();
        }
    }

    void ClientContext::SaveKeyValue(const std::string& k, const std::string& v) {
        sp_->Put(k, v);
    }

    std::string ClientContext::GetValueByKey(const std::string& k) {
        return sp_->Get(k);
    }

    bool ClientContext::IsRender() {
        return render_;
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

    int ClientContext::GetCapturingMonitorIndex() {
        return capturing_info_.mon_idx_;
    }

}
//
// Created by RGAA on 2024-04-20.
//

#include "render_service_client.h"
#include "rd_context.h"
#include "rd_statistics.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include "rd_app.h"
#include "app/app_messages.h"
#include "tc_service_message.pb.h"
#include "settings/settings.h"

namespace tc
{

    const int kMaxClientQueuedMessage = 4096;

    RenderServiceClient::RenderServiceClient(const std::shared_ptr<RdApplication>& app) {
        statistics_ = RdStatistics::Instance();
        app_ = app;
        context_ = app_->GetContext();
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgTimer1000>([=, this](const MsgTimer1000& msg) {
            this->HeartBeat();
        });
    }

    void RenderServiceClient::Start() {
        client_ = std::make_shared<asio2::ws_client>();
        client_->set_auto_reconnect(true);
        client_->keep_alive(true);
        client_->set_timeout(std::chrono::milliseconds(3000));

        client_->bind_init([&]() {
            client_->ws_stream().binary(true);
            client_->set_no_delay(true);

        }).bind_connect([&]() {
            if (asio2::get_last_error()) {
                LOGE("connect failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
            } else {
                LOGI("connect success : {} {} ", client_->local_address().c_str(), client_->local_port());
            }

            context_->PostTask([=, this]() {
                context_->SendAppMessage(MsgRenderConnected2Service{});
            });

        }).bind_upgrade([&]() {
            if (asio2::get_last_error()) {
                LOGE("upgrade failure : {}, {}", asio2::last_error_val(), asio2::last_error_msg());
            }
        }).bind_recv([=, this](std::string_view data) {
            auto msg = std::string(data.data(), data.size());
            this->ParseMessage(msg);
        });

        // the /ws is the websocket upgraged target
        if (!client_->start("127.0.0.1", 20375, "/service/message?from=panel")) {
            LOGE("connect websocket server failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }
    }

    void RenderServiceClient::ParseMessage(const std::string& msg) {
        tc::ServiceMessage sm;
        try {
            sm.ParseFromString(msg);
        }
        catch(...) {
            LOGE("ParseMessage failed!");
            return;
        }

        if (sm.type() == ServiceMessageType::kSrvHeartBeatResp) {
            auto sub = sm.heart_beat_resp();
            auto hb_idx = sub.index();
            auto is_render_alive = sub.render_status() == RenderStatus::kWorking;
            //LOGI("hb_idx: {}, is render alive: {}", hb_idx, is_render_alive);
        }
    }

    void RenderServiceClient::Exit() {
        if (client_) {
            client_->stop();
        }
    }

    bool RenderServiceClient::IsAlive() {
        return client_ && client_->is_started();
    }

    void RenderServiceClient::HeartBeat() {
        static int64_t hb_idx = 0;
        tc::ServiceMessage msg;
        msg.set_type(ServiceMessageType::kSrvHeartBeat);
        auto sub = msg.mutable_heart_beat();
        sub->set_index(hb_idx++);
        sub->set_from(std::format("render_{}", Settings::Instance()->transmission_.listening_port_));
        PostNetMessage(msg.SerializeAsString());
    }

    void RenderServiceClient::PostNetMessage(const std::string& msg) {
        if (client_ && client_->is_started()) {
            if (queued_msg_count_ > kMaxClientQueuedMessage) {
                LOGW("too many message in queue, discard the message in RenderServiceClient");
                return;
            }
            queued_msg_count_++;
            client_->async_send(msg, [=, this]() {
                queued_msg_count_--;
            });
        }
    }

}
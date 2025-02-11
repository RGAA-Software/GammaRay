//
// Created by hy RGAA
//
#include "sig_sdk_ws_router.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include "sig_sdk_maker.h"
#include "sig_sdk_apis.h"
#include "sig_sdk_events.h"

namespace tc
{

    SigSdkWsRouter::SigSdkWsRouter(const std::shared_ptr<SigSdkContext>& ctx) : SigSdkAbsRouter(ctx) {

    }

    void SigSdkWsRouter::Init(const SignalingParam& params) {
        sig_params_ = params;
    }

    void SigSdkWsRouter::Start() {
        std::string host = sig_params_.host_;
        int port = sig_params_.port_;
        std::string path = kApiWsSig;
        client_ = std::make_shared<asio2::ws_client>();
        client_->set_connect_timeout(std::chrono::seconds(5));
        client_->set_auto_reconnect(true, std::chrono::seconds(3));
        int timer1s = 1000;
        client_->start_timer(std::to_string(timer1s), std::chrono::milliseconds(timer1s), [this]() {
            SigEvtTimer1S evt;
            msg_notifier_->SendAppMessage(evt);
        });

        int timer2s = 2000;
        client_->start_timer(std::to_string(timer2s), std::chrono::milliseconds(timer2s), [this]() {
            this->SendHeartBeat();

            SigEvtTimer2S evt;
            msg_notifier_->SendAppMessage(evt);
        });

        int timer5s = 5000;
        client_->start_timer(std::to_string(timer5s), std::chrono::milliseconds(timer5s), [this]() {
            msg_notifier_->SendAppMessage(SigEvtTimer5S{});
        });

        client_->bind_init([&]() {
            client_->ws_stream().binary(true);
        });

        client_->bind_connect([=]() {
            bool success = false;
            if (asio2::get_last_error()) {
                std::string err_msg = asio2::last_error_msg();
    #ifdef WIN32
                //err_msg = GbkToUtf8(err_msg);
    #endif
                LOGE("connect failure : {} {}", asio2::last_error_val(), err_msg);
            } else {
                LOGI("connect success : {} {}", client_->local_address().c_str(), client_->local_port());
                success = true;
            }
            asio2::clear_last_error();

            SigEvtOnConnect evt;
            evt.connected_ = success;
            msg_notifier_->SendAppMessage(evt);
        });

        client_->bind_disconnect([=]() {
            SigEvtOnDisconnect evt;
            msg_notifier_->SendAppMessage(evt);
        });

        client_->bind_upgrade([&]() {
            if (asio2::get_last_error()) {
                LOGE("update failure: {}, {}", asio2::last_error_val(), asio2::last_error_msg());
            } else {
                const websocket::response_type& rep = client_->get_upgrade_response();
                auto it = rep.find(http::field::authentication_results);
                if (it != rep.end()) {
                    beast::string_view auth = it->value();
                    std::cout << auth << std::endl;
                    ASIO2_ASSERT(auth == "200 OK");
                }
            }
        });

        client_->bind_recv([&](std::string_view data) {
            if (sig_msg_cbk_) {
                std::string cpy_data(data.data(), data.size());
                sig_msg_cbk_(cpy_data);
            }
        });

        if (!client_->async_start(host, port, path)) {
            LOGE("connect websocket server failure : {}, {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }
    }

    void SigSdkWsRouter::Exit() {
        if (IsAlive()) {
            client_->stop_all_timers();
            client_->stop();
        }
    }

    void SigSdkWsRouter::SendSigMessage(const std::string& sig_name, const std::string& token, const std::string& msg) {
        if (!IsAlive()) {
            return;
        }
        client_->async_send(msg);
    }

    void SigSdkWsRouter::SendHeartBeat() {
        if (!IsAlive()) {
            return;
        }
        SigHeartBeatMessage msg;
        msg.sig_name_ = kSigHeartBeat;
        msg.client_id_ = rtc_ctx_->GetClientId();
        msg.index_ = heart_beat_idx_++;
        msg.local_ips_ = rtc_ctx_->GetLocalIps();
        msg.www_ips_ = rtc_ctx_->GetWwwIps();
        auto sig_msg = SigSdkMessageMaker::MakeHeartBeat(msg);
        client_->async_send(sig_msg);
    }

    bool SigSdkWsRouter::IsAlive() {
        bool alive = client_ && client_->is_started();
        if (!alive) {
            LOGE("client not ready: Method:{} Line:{}", __FUNCTION__, __LINE__);
        }
        return alive;
    }

}
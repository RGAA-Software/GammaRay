//
// Created by RGAA on 17/05/2025.
//

#include "gr_spvr_client.h"
#include "spvr_panel.pb.h"
#include "tc_common_new/log.h"
#include "render_panel/gr_context.h"
#include "tc_common_new/message_notifier.h"
#include "render_panel/gr_app_messages.h"
#include "tc_common_new/time_util.h"
#include "spvr_panel.pb.h"
#include "render_panel/gr_settings.h"
#include "tc_common_new/base64.h"
#include "hw_info/hw_info.h"

using namespace spvr_panel;

namespace tc
{
    GrSpvrClient::GrSpvrClient(const std::shared_ptr<GrContext>& ctx,
                               const std::string& host,
                               int port,
                               const std::string& device_id,
                               const std::string& appkey) {
        settings_ = GrSettings::Instance();
        context_ = ctx;
        host_ = host;
        port_ = port;
        device_id_ = device_id;
        appkey_ = appkey;

        msg_listener_ = context_->ObtainMessageListener();
        msg_listener_->Listen<MsgGrTimer1S>([=, this](const MsgGrTimer1S& m) {
            context_->PostTask([=, this]() {
                this->Heartbeat();
            });
        });

        msg_listener_->Listen<MsgHWInfo>([=, this](const MsgHWInfo& info) {
            sys_info_ = info.sys_info_;
        });

    }

    void GrSpvrClient::Start() {
        client_ = std::make_shared<asio2::wss_client>();
        client_->set_auto_reconnect(true);
        client_->keep_alive(true);
        client_->set_timeout(std::chrono::milliseconds(3000));
        client_->set_verify_mode(asio::ssl::verify_none);

        client_->bind_init([&]() {
            client_->ws_stream().binary(true);
            client_->set_no_delay(true);
        })
        .bind_connect([=, this]() {
            if (asio2::get_last_error()) {
                LOGE("connect failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
            } else {
                LOGI("connect success : {} {} ", client_->local_address().c_str(), client_->local_port());
            }

            client_->post_queued_event([=, this]() {
                this->Hello();
            });

        })
        .bind_upgrade([=, this]() {
            if (asio2::get_last_error()) {
                LOGE("upgrade failure : {}, {}", asio2::last_error_val(), asio2::last_error_msg());
            }
        })
        .bind_disconnect([=, this]() {
            LOGE("*** Disconnected for spvr-client: {}", device_id_);
        })
        .bind_recv([=, this](std::string_view data) {
            auto msg = std::string(data.data(), data.size());
            this->ParseMessage(msg);
        });

        // the /ws is the websocket upgraged target
        auto path = std::format("/spvr/panel?appkey={}&device_id={}", appkey_, device_id_);
        LOGI("will connect => {}:{}{}", host_, port_, path);
        if (!client_->async_start(host_, port_, path)) {
            LOGE("connect websocket server failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }
    }

    bool GrSpvrClient::IsStarted() {
        return client_ != nullptr;
    }

    bool GrSpvrClient::IsActive() {
        return IsStarted() && client_->is_started();
    }

    void GrSpvrClient::Hello() {
        if (!IsActive()) {
            return;
        }
        spvr_panel::SpvrPanelMessage msg;
        msg.set_msg_type(spvr_panel::SpvrPanelMessageType::kSpvrPanelHello);
        auto sub = msg.mutable_hello();
        sub->set_device_id(device_id_);
        PostBinMessage(msg.SerializeAsString());
    }

    void GrSpvrClient::Heartbeat() {
        if (!IsActive()) {
            return;
        }

        auto desktop_link_raw = context_->MakeBroadcastMessage();
        auto desktop_link = std::format("link://{}", Base64::Base64Encode(desktop_link_raw));

        spvr_panel::SpvrPanelMessage msg;
        msg.set_msg_type(spvr_panel::SpvrPanelMessageType::kSpvrPanelHeartBeat);
        auto sub = msg.mutable_heartbeat();
        sub->set_hb_index(hb_idx_++);
        sub->set_device_id(device_id_);
        sub->set_desktop_link(desktop_link);
        sub->set_desktop_link_raw(desktop_link_raw);
        if (auto sys_info = sys_info_.Load(); sys_info != nullptr) {
            sub->set_sys_info_raw(sys_info->raw_json_msg_);
        }
        PostBinMessage(msg.SerializeAsString());
    }

    void GrSpvrClient::PostBinMessage(const std::string& m) {
        if (IsActive()) {
            client_->async_send(m);
        }
    }

    void GrSpvrClient::ParseMessage(const std::string& m) {
        auto pm = std::make_shared<spvr_panel::SpvrPanelMessage>();
        bool r = pm->ParsePartialFromString(m);
        if (!r) {
            LOGE("Parse SpvrClient message failed!");
            return;
        }
        last_received_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();

        auto type = pm->msg_type();
        if (type == SpvrPanelMessageType::kSpvrPanelHello) {
            LOGI("SpvrClient hello.");
        }
        else if (type == SpvrPanelMessageType::kSpvrPanelHeartBeat) {
            //LOGI("SpvrClient heartbeat.");
        }
    }

    bool GrSpvrClient::IsAlive() const {
        auto current_timestamp = TimeUtil::GetCurrentTimestamp();
        auto diff = current_timestamp - last_received_timestamp_ < 3100;
        //LOGI("Diff alive: {}", diff);
        return diff;
    }

}
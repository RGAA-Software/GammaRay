//
// Created by RGAA on 17/05/2025.
//

#include "ct_panel_client.h"
#include "ct_settings.h"
#include "tc_common_new/log.h"
#include "ct_client_context.h"
#include "tc_client_panel_message.pb.h"
#include "tc_client_sdk_new/sdk_statistics.h"
#include "tc_common_new/message_notifier.h"
#include "tc_client_panel_message.pb.h"

namespace tc
{

    CtPanelClient::CtPanelClient(const std::shared_ptr<ClientContext>& ctx) {
        context_ = ctx;
        msg_listener_ = ctx->ObtainMessageListener();
        msg_listener_->Listen<MsgClientFileTransmissionBegin>([=, this](const MsgClientFileTransmissionBegin& msg) {
            context_->PostTask([=, this]() {
                this->ReportFileTransferBegin(msg);
            });
        });

        msg_listener_->Listen<MsgClientFileTransmissionEnd>([=, this](const MsgClientFileTransmissionEnd& msg) {
            context_->PostTask([=, this]() {
                this->ReportFileTransferEnd(msg);
            });
        });
    }

    void CtPanelClient::Start() {
        client_ = std::make_shared<asio2::ws_client>();
        client_->set_auto_reconnect(true);
        client_->keep_alive(true);
        client_->set_timeout(std::chrono::milliseconds(3000));
        client_->start_timer("timer_1000", 1000, [=, this]() {
            this->HeartBeat();
        });

        client_->bind_init([&]() {
            client_->ws_stream().binary(true);
            client_->set_no_delay(true);

        }).bind_connect([=, this]() {
            if (asio2::get_last_error()) {
                LOGE("connect failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
            } else {
                LOGI("connect success : {} {} ", client_->local_address().c_str(), client_->local_port());
            }

            client_->post_queued_event([=, this]() {
                this->Hello();
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
        auto settings = Settings::Instance();
        auto path = std::format("/panel?stream_id={}", settings->stream_id_);
        if (!client_->async_start("127.0.0.1", settings->panel_server_port_, path)) {
            LOGE("connect websocket server failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }
    }

    bool CtPanelClient::IsAlive() {
        return client_ && client_->is_started();
    }

    void CtPanelClient::ParseMessage(std::string_view data) {

    }

    void CtPanelClient::Hello() {
        if (!IsAlive()) {
            return;
        }
        auto settings = Settings::Instance();

        tccp::CpMessage cp_msg;
        cp_msg.set_type(tccp::CpMessageType::kCpHello);
        cp_msg.set_stream_id(settings->stream_id_);
        auto sub = cp_msg.mutable_hello();
#ifdef WIN32
        sub->set_type(tccp::CpSessionType::kWindowsClient);
#endif
        client_->async_send(cp_msg.SerializeAsString());
    }

    void CtPanelClient::HeartBeat() {
        if (!IsAlive()) {
            return;
        }

        auto stat = tc::SdkStatistics::Instance();
        auto settings = Settings::Instance();

        tccp::CpMessage cp_msg;
        cp_msg.set_type(tccp::CpMessageType::kCpHeartBeat);
        cp_msg.set_stream_id(settings->stream_id_);
        auto sub = cp_msg.mutable_heartbeat();
        sub->set_remote_device_desktop_name(stat->remote_desktop_name_.Clone());
        sub->set_remote_os_name(stat->remote_os_name_.Clone());
        client_->async_send(cp_msg.SerializeAsString());
    }

    void CtPanelClient::ReportFileTransferBegin(const MsgClientFileTransmissionBegin& msg) {
        if (!IsAlive()) {
            return;
        }
        auto settings = Settings::Instance();
        tccp::CpMessage cp_msg;
        cp_msg.set_type(tccp::CpMessageType::kCpFileTransferBegin);
        cp_msg.set_stream_id(settings->stream_id_);
        auto sub = cp_msg.mutable_ft_transfer_beg();
        sub->set_the_file_id(msg.the_file_id_);
        sub->set_begin_timestamp(msg.begin_timestamp_);
        sub->set_direction(msg.direction_);
        sub->set_file_detail(msg.file_detail_);
        sub->set_remote_device_id(msg.remote_device_id_);
        client_->async_send(cp_msg.SerializeAsString());
    }

    void CtPanelClient::ReportFileTransferEnd(const MsgClientFileTransmissionEnd& msg) {
        if (!IsAlive()) {
            return;
        }
        auto settings = Settings::Instance();
        tccp::CpMessage cp_msg;
        cp_msg.set_type(tccp::CpMessageType::kCpFileTransferEnd);
        cp_msg.set_stream_id(settings->stream_id_);
        auto sub = cp_msg.mutable_ft_transfer_end();
        sub->set_the_file_id(msg.the_file_id_);
        sub->set_end_timestamp(msg.end_timestamp_);
        sub->set_success(msg.success_);
        client_->async_send(cp_msg.SerializeAsString());
    }

}
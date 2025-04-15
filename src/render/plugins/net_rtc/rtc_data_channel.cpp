//
// Created by RGAA
//

#include "rtc_data_channel.h"
#include "tc_common_new/log.h"
#include "rtc_server.h"

namespace tc
{

    RtcDataChannel::RtcDataChannel(const std::string& name, const std::shared_ptr<RtcServer>& rtc_server, rtc::scoped_refptr<webrtc::DataChannelInterface> ch) {
        this->name_ = name;
        this->rtc_server_ = rtc_server;
        this->plugin_ = rtc_server->GetPlugin();
        this->data_channel_ = ch;
        this->data_channel_->RegisterObserver(this);
    }

    RtcDataChannel::~RtcDataChannel() {
        this->data_channel_->UnregisterObserver();
    }

    void RtcDataChannel::OnStateChange() {
//        if (exit_via_reconnect_) {
//            return;
//        }
        if (data_channel_->state() == webrtc::DataChannelInterface::kOpen) {
            connected_ = true;
            //context_->SendAppMessage(ClientEvtDataChannelReady{});

        } else if (data_channel_->state() == webrtc::DataChannelInterface::kClosed) {
            connected_ = false;
            //context_->SendAppMessage(ClientEvtDataChannelClosed{});

        }
        LOGI("DataChannel state changed: {}, connected: {}", data_channel_->state(), connected_);
    }

    void RtcDataChannel::OnMessage(const webrtc::DataBuffer &buffer) {
        //RLogI("DataChannel Message: {}", buffer.size());
        std::string event((char*)buffer.data.data(), buffer.size());
        //ClientEvtFromDataChannel evt;
        //evt.event_ = event;
        //context_->SendAppMessage(evt);
    }

    void RtcDataChannel::OnBufferedAmountChange(uint64_t sent_data_size) {
        DataChannelObserver::OnBufferedAmountChange(sent_data_size);
    }

    bool RtcDataChannel::IsConnected() {
        return connected_;
    }

    void RtcDataChannel::SendData(const std::string& msg) {
        if (!connected_) {
            LOGW("DataChannel is not connected now, channel name: {}", name_);
            return;
        }
        ++pending_data_count_;
        this->data_channel_->Send(webrtc::DataBuffer(msg));
        //RLogI("send data via data channel: {}", msg.size());
        --pending_data_count_;

        //LOGI("send pending count: {}", pending_data_count_);
    }

    int RtcDataChannel::GetPendingDataCount() {
        return pending_data_count_;
    }

    void RtcDataChannel::Close() {
        connected_ = false;
        if (data_channel_) {
            data_channel_->Close();
        }
    }

} // namespace dl
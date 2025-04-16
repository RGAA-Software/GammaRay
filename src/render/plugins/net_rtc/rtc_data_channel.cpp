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

        LOGI("DataChannel[ {} ] state changed: {}, connected: {}", name_, (int)data_channel_->state(), connected_);
    }

    void RtcDataChannel::OnMessage(const webrtc::DataBuffer &buffer) {
        //LOGI("from: {}=> Message: {}", name_, buffer.size());
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
            //LOGW("DataChannel is invalid, name: {}", name_);
            return;
        }
        ++pending_data_count_;
        //auto total_size = data_channel_->buffered_amount();
        //LOGI("buffered: {}", total_size);
        //if (!this->data_channel_->Send(webrtc::DataBuffer(msg))) {
        //    LOGE("Send data failed, size: {}", msg.size());
        //}
        auto kB_size = msg.size()/1024;
        if (kB_size > 15) {
            LOGI("send data via data channel: {}", kB_size);
        }
        data_channel_->SendAsync(webrtc::DataBuffer(rtc::CopyOnWriteBuffer(msg),true), [=, this](webrtc::RTCError err) {
            --pending_data_count_;
            if (!err.ok()) {
                LOGE("SendAsync error: {}", err.message());
            }
        });

        //LOGI("send pending count: {}", pending_data_count_);
    }

    int RtcDataChannel::GetPendingDataCount() {
        return pending_data_count_;
    }

    void RtcDataChannel::Close() {
        LOGI("DataChannel will close!");
        connected_ = false;
        if (data_channel_) {
            data_channel_->Close();
        }
    }

} // namespace dl
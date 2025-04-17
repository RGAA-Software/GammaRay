//
// Created by RGAA
//

#include "rtc_data_channel.h"
#include "tc_common_new/log.h"
#include "rtc_server.h"
#include "tc_common_new/net_tlv_header.h"

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
        std::string data((char*)buffer.data.data(), buffer.size());
        if (data_cbk_) {
            data_cbk_(std::move(data));
        }
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

    void RtcDataChannel::SetOnDataCallback(OnDataCallback&& cbk) {
        data_cbk_ = cbk;
    }

    void RtcDataChannel::SendData(const std::string& msg) {
        if (!connected_) {
            //LOGW("DataChannel is invalid, name: {}", name_);
            return;
        }

        if (msg.size() <= kSplitBufferSize) {
            // wrap message
            auto header = NetTlvHeader {
                .type_ = kNetTlvFull,
                .this_buffer_length_ = (uint32_t)msg.size(),
                .this_buffer_begin_ = 0,
                .this_buffer_end_ = (uint32_t)msg.size(),
                .parent_buffer_length_ = (uint32_t)msg.size(),
            };

            std::string buffer;
            buffer.resize(sizeof(NetTlvHeader) + msg.size());
            memcpy((char*)buffer.data(), (char*)&header, sizeof(NetTlvHeader));
            memcpy((char*)buffer.data() + sizeof(NetTlvHeader), msg.data(), msg.size());

            ++pending_data_count_;
            auto rtc_buffer = webrtc::DataBuffer(rtc::CopyOnWriteBuffer(buffer), true);
            data_channel_->SendAsync(rtc_buffer, [=, this](webrtc::RTCError err) {
                 --pending_data_count_;
                 if (!err.ok()) {
                     LOGE("SendAsync error: {}", err.message());
                 }
             });
        }
        else {
            auto size = (uint32_t)msg.size();
            auto pieces = [&]() {
                auto full_part_size = size / kSplitBufferSize;
                auto left_size = size % kSplitBufferSize;
                if (left_size == 0) {
                    return full_part_size;
                }
                else {
                    return full_part_size + 1;
                }
            }();

            LOGI("[ {} ]message size: {}KB, to pieces: {}, base: {}", name_, msg.size()/1024, pieces, kSplitBufferSize);

            auto total_size = (uint32_t)msg.size();
            for (int i = 0; i < pieces; i++) {
                auto type = kNetTlvBegin;
                if (i == 0) {
                    type = kNetTlvBegin;
                }
                else if (i == (pieces-1)) {
                    type = kNetTlvEnd;
                }
                else {
                    type = kNetTlvCenter;
                }

                auto this_buffer_begin = i * kSplitBufferSize;
                auto this_buffer_end = std::min(total_size, this_buffer_begin + kSplitBufferSize);
                auto this_buffer_length = this_buffer_end - this_buffer_begin;
                auto header = NetTlvHeader {
                    .type_ = type,
                    .this_buffer_length_ = this_buffer_length,
                    .this_buffer_begin_ = this_buffer_begin,
                    .this_buffer_end_ = this_buffer_end,
                    .parent_buffer_length_ = total_size,
                };

                LOGI("[ {} ]send buffer from: {} => {}, size: {}, total: {}", name_, this_buffer_begin, this_buffer_end, this_buffer_length, total_size);

                std::string buffer;
                buffer.resize(sizeof(NetTlvHeader) + this_buffer_length);
                memcpy((char*)buffer.data(), (char*)&header, sizeof(NetTlvHeader));
                memcpy((char*)buffer.data() + sizeof(NetTlvHeader), (char*)msg.data()+this_buffer_begin, this_buffer_length);

                ++pending_data_count_;
                auto rtc_buffer = webrtc::DataBuffer(rtc::CopyOnWriteBuffer(buffer), true);
                data_channel_->SendAsync(rtc_buffer, [=, this](webrtc::RTCError err) {
                    --pending_data_count_;
                    if (!err.ok()) {
                        LOGE("SendAsync error: {}", err.message());
                    }
                });
            }
        }

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
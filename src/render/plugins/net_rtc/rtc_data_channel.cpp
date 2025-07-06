//
// Created by RGAA
//

#include "rtc_data_channel.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "rtc_server.h"
#include "tc_common_new/net_tlv_header.h"
#include "tc_common_new/time_util.h"
#include "plugin_interface/gr_plugin_context.h"
#include "plugin_interface/gr_plugin_events.h"
#include "rtc_plugin.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/uuid.h"

namespace tc
{

    RtcDataChannel::RtcDataChannel(const std::string& name, const std::shared_ptr<RtcServer>& rtc_server, rtc::scoped_refptr<webrtc::DataChannelInterface> ch) {
        this->name_ = name;
        this->rtc_server_ = rtc_server;
        this->plugin_ = rtc_server->GetPlugin();
        this->plugin_ctx_ = this->plugin_->GetPluginContext();
        this->data_channel_ = ch;
        this->data_channel_->RegisterObserver(this);
        this->the_conn_id_ = MD5::Hex(tc::GetUUID());
        this->created_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
    }

    RtcDataChannel::~RtcDataChannel() {
        this->data_channel_->UnregisterObserver();
    }

    void RtcDataChannel::OnStateChange() {
        if (data_channel_->state() == webrtc::DataChannelInterface::kOpen) {
            connected_ = true;

            // notify
            if (this->name_ == "media_data_channel") {
                auto event = std::make_shared<GrPluginClientConnectedEvent>();
                event->stream_id_ = this->the_conn_id_;
                event->visitor_device_id_ = name_;
                event->conn_type_ = "P2P";
                event->begin_timestamp_ = created_timestamp_;
                this->plugin_->CallbackEvent(event);
            }

        } else if (data_channel_->state() == webrtc::DataChannelInterface::kClosed) {
            connected_ = false;
        }

        LOGI("DataChannel[ {} ] state changed: {}, connected: {}", name_, (int)data_channel_->state(), connected_);
    }

    void RtcDataChannel::OnMessage(const webrtc::DataBuffer &buffer) {
        auto header = (NetTlvHeader*)buffer.data.data();
        std::string data;
        data.resize(header->this_buffer_length_);
        memcpy(data.data(), (char*)header + sizeof(NetTlvHeader), header->this_buffer_length_);

        auto curr_timestamp = tc::TimeUtil::GetCurrentTimestamp();
        auto diff_time = curr_timestamp - last_recv_msg_timestamp_;
        last_recv_msg_timestamp_ = curr_timestamp;

        if (IsFtChannel()) {
            //LOGI("from: {}, index: {} => Message size: {}", name_, header->pkt_index_, header->this_buffer_length_);
            auto curr_pkt_index = header->pkt_index_;
            if (last_recv_pkt_index_ == 0) {
                last_recv_pkt_index_ = curr_pkt_index;
            }
            auto diff = curr_pkt_index - last_recv_pkt_index_;
            if (diff > 1) {
                //LOGE("**** Message Index Error ****\n current index: {}, last index: {}", curr_pkt_index, last_recv_pkt_index_);
            }
            last_recv_pkt_index_ = curr_pkt_index;

            std::lock_guard<std::mutex> guard(cached_messages_mtx_);
            cached_ft_messages_.insert({header->pkt_index_, data});

        }
        else {
            if (header->type_ == kNetTlvFull) {
                if (data_cbk_) {
                    data_cbk_(data);
                }
            }
        }
    }

    void RtcDataChannel::On100msTimeout() {
        this->plugin_->PostWorkTask([=, this]() {
            std::lock_guard<std::mutex> guard(cached_messages_mtx_);
            uint64_t beg_idx = 0;
            bool lack_messages = false;
            for (const auto& [k, data] : cached_ft_messages_) {
                if (beg_idx == 0) {
                    beg_idx = k;
                }
                if (k - beg_idx > 1) {
                    lack_messages = true;
                    break;
                }
                beg_idx = k;
            }

            if (lack_messages) {
                LOGW("Lack messages! cached message size: {}", cached_ft_messages_.size());
                std::stringstream ss;
                for (const auto& [k, data] : cached_ft_messages_) {
                    ss << k << ",";
                }
                LOGW("cached message sort: {}", ss.str());
                if (cached_ft_messages_.size() > 1024*8) {
                    // clear it
                    LOGE("Clear all cached messages, count: {}", cached_ft_messages_.size());
                    cached_ft_messages_.clear();

                    // TODO: Notify error
                }
                return;
            }

            //LOGI("Cached message size: {}", cached_ft_messages_.size());
            for (const auto& [k, data] : cached_ft_messages_) {
                if (data_cbk_) {
                    data_cbk_(data);
                }
            }
            cached_ft_messages_.clear();
        });
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

    void RtcDataChannel::SendData(std::shared_ptr<Data> msg) {
        if (!connected_) {
            LOGW("DataChannel is invalid, name: {}", name_);
            return;
        }

        total_send_content_bytes_ += msg->Size();

        if (msg->Size() <= kSplitBufferSize) {
            // wrap message
            auto header = NetTlvHeader {
                .type_ = kNetTlvFull,
                .this_buffer_length_ = (uint32_t)msg->Size(),
                .this_buffer_begin_ = 0,
                .this_buffer_end_ = (uint32_t)msg->Size(),
                .pkt_index_ = send_pkt_index_++,
                .parent_buffer_length_ = (uint32_t)msg->Size(),
            };

            std::string buffer;
            buffer.resize(sizeof(NetTlvHeader) + msg->Size());
            memcpy((char*)buffer.data(), (char*)&header, sizeof(NetTlvHeader));
            memcpy((char*)buffer.data() + sizeof(NetTlvHeader), msg->CStr(), msg->Size());

            ++pending_data_count_;
            auto rtc_buffer = webrtc::DataBuffer(rtc::CopyOnWriteBuffer(buffer), true);

            // test beg //
            auto buffered_amount = data_channel_->buffered_amount();
            auto max_queue_size = data_channel_->MaxSendQueueSize();
            if (buffered_amount >= max_queue_size/2) {
                //LOGW("buffered amount: {}, max queue size: {}", buffered_amount, max_queue_size);
            }
            // test //
            //if (IsFtChannel()) {
            //    LOGI("Send Pkt index: {}, send_content_bytes: {}", header.pkt_index_, total_send_content_bytes_);
            //}
            // test end //

            bool ok = data_channel_->Send(rtc_buffer);
            if (!ok) {
                LOGE("Send error in channel: {}", name_);
                pending_data_count_ = 0;
                connected_ = false;
                // TODO: Notify
            }
            else {
                --pending_data_count_;
            }

            if (pending_data_count_ > 80) {
                LOGI("[ {} ] pending_data_count: {}", name_, pending_data_count_);
            }
        }
        else {
            auto size = (uint32_t)msg->Size();
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

            if (IsMediaChannel()) {
                //LOGI("[ {} ]message size: {}KB, to pieces: {}, base: {}", name_, msg.size() / 1024, pieces, kSplitBufferSize);
            }

            auto total_size = (uint32_t)msg->Size();
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
                    .pkt_index_ = send_pkt_index_++,
                    .parent_buffer_length_ = total_size,
                };

                if (IsMediaChannel()) {
                    //LOGI("[ {} ]send buffer from: {} => {}, size: {}, total: {}", name_, this_buffer_begin,
                    //     this_buffer_end, this_buffer_length, total_size);
                }
                std::string buffer;
                buffer.resize(sizeof(NetTlvHeader) + this_buffer_length);
                memcpy((char*)buffer.data(), (char*)&header, sizeof(NetTlvHeader));
                memcpy((char*)buffer.data() + sizeof(NetTlvHeader), (char*)msg->CStr()+this_buffer_begin, this_buffer_length);

                ++pending_data_count_;
                auto rtc_buffer = webrtc::DataBuffer(rtc::CopyOnWriteBuffer(buffer), true);
                bool ok = data_channel_->Send(rtc_buffer);
                if (!ok) {
                    LOGE("Send error in channel: {}", name_);
                    pending_data_count_ = 0;
                    connected_ = false;
                    // TODO: Notify
                }
                else {
                    --pending_data_count_;
                }
            }
        }

        //LOGI("send pending count: {}", pending_data_count_);
    }

    int RtcDataChannel::GetPendingDataCount() {
        return pending_data_count_;
    }

    bool RtcDataChannel::HasEnoughBufferForQueuingMessages() {
        return data_channel_
               && data_channel_->state() == webrtc::DataChannelInterface::DataState::kOpen
               && data_channel_->buffered_amount() <= data_channel_->MaxSendQueueSize()*1/4;
    }

    bool RtcDataChannel::IsMediaChannel() {
        return name_ == "media_data_channel";
    }

    bool RtcDataChannel::IsFtChannel() {
        return name_ == "ft_data_channel";
    }

    void RtcDataChannel::Close() {
        LOGI("DataChannel will close!");
        connected_ = false;
        if (data_channel_) {
            data_channel_->Close();
        }
    }

} // namespace dl
//
// Created by RGAA
//

#ifndef TC_PLUGIN_DATA_CHANNEL_OBSERVER_H
#define TC_PLUGIN_DATA_CHANNEL_OBSERVER_H

#include <atomic>
#include <mutex>
#include <map>
#include "tc_common_new/webrtc_helper.h"

namespace tc
{
    using OnDataCallback = std::function<void(const std::string&)>;

    class RtcServer;
    class RtcPlugin;
    class GrPluginContext;

    class RtcDataChannel :  public webrtc::DataChannelObserver {
    public:
        RtcDataChannel(const std::string& name, const std::shared_ptr<RtcServer>& rtc_server, rtc::scoped_refptr<webrtc::DataChannelInterface> ch);
        ~RtcDataChannel() override;

        void OnStateChange() override;
        void OnMessage(const webrtc::DataBuffer& buffer) override;
        void OnBufferedAmountChange(uint64_t sent_data_size) override;
        bool IsConnected();
        void SendData(const std::string& data);
        int GetPendingDataCount();
        void Close();

        void SetOnDataCallback(OnDataCallback&&);
        bool HasEnoughBufferForQueuingMessages();

        void On100msTimeout();

    private:
        bool IsMediaChannel();
        bool IsFtChannel();

    private:
        RtcPlugin* plugin_;
        std::shared_ptr<GrPluginContext> plugin_ctx_ = nullptr;
        std::string name_;
        rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_ = nullptr;
        std::shared_ptr<RtcServer> rtc_server_ = nullptr;
        std::atomic<bool> connected_ = false;
        std::atomic<int> pending_data_count_ = 0;
        std::atomic<uint64_t> send_pkt_index_ = 0;
        OnDataCallback data_cbk_;

        std::atomic<uint64_t> last_recv_pkt_index_ = 0;
        uint64_t total_send_content_bytes_ = 0;
        uint64_t last_recv_msg_timestamp_ = 0;

        std::mutex cached_messages_mtx_;
        std::map<uint64_t, std::string> cached_ft_messages_;

    public:
        std::string the_conn_id_;
        int64_t created_timestamp_{0};
    };

} // namespace dl

#endif //TEST_WEBRTC_DATA_CHANNEL_OBSERVER_IMPL_H

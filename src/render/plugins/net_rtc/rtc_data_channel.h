//
// Created by RGAA
//

#ifndef TC_PLUGIN_DATA_CHANNEL_OBSERVER_H
#define TC_PLUGIN_DATA_CHANNEL_OBSERVER_H

#include <atomic>
#include "tc_common_new/webrtc_helper.h"

namespace tc
{

    class RtcServer;
    class RtcPlugin;

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
        void SetExitViaReconnect(bool reconn) { exit_via_reconnect_ = reconn; }
        void Close();

    private:
        RtcPlugin* plugin_;
        std::string name_;
        rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_ = nullptr;
        std::shared_ptr<RtcServer> rtc_server_ = nullptr;
        std::atomic<bool> connected_ = false;
        std::atomic<int> pending_data_count_ = 0;  //只是表达排队数,并没有关心data发送是否成功
        bool exit_via_reconnect_ = false;
    };

} // namespace dl

#endif //TEST_WEBRTC_DATA_CHANNEL_OBSERVER_IMPL_H

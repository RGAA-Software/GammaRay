//
// Created by RGAA on 13/04/2025.
//

#ifndef GAMMARAY_RTC_CONNECTION_H
#define GAMMARAY_RTC_CONNECTION_H

#include "rtc_client_interface.h"
#include "tc_common_new/webrtc_helper.h"
#include <memory>
#include <mutex>

extern "C" __declspec(dllexport) void* GetInstance();

namespace tc
{

    class PeerCallback;
    class SetSessCallback;
    class CreateSessCallback;
    class RtcDataChannel;

    // cached ice
    class CachedIce {
    public:
        std::string ice_;
        std::string mid_;
        int sdp_mline_index_{0};
    };

    class RtcConnection : public RtcClientInterface {
    public:
        RtcConnection();
        bool Init() override;
        bool OnRemoteSdp(const std::string &sdp) override;
        bool OnRemoteIce(const std::string &ice, const std::string &mid, int32_t sdp_mline_index) override;

    private:
        void CreatePeerConnection();
        void CreatePeerConnectionFactory();

        void SendCachedIces();

    private:
        std::shared_ptr<PeerCallback> peer_callback_ = nullptr;
        rtc::scoped_refptr<SetSessCallback> set_local_sdp_callback_ = nullptr;
        rtc::scoped_refptr<SetSessCallback> set_remote_sdp_callback_ = nullptr;
        rtc::scoped_refptr<CreateSessCallback> create_sess_callback_ = nullptr;

        std::unique_ptr<rtc::Thread> network_thread_;
        std::unique_ptr<rtc::Thread> worker_thread_;
        std::unique_ptr<rtc::Thread> sig_thread_;

        rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_conn_ = nullptr;
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_conn_factory_;
        webrtc::PeerConnectionInterface::RTCConfiguration configuration_;
        std::shared_ptr<RtcDataChannel> media_data_channel_ = nullptr;
        std::shared_ptr<RtcDataChannel> ft_data_channel_ = nullptr;

        std::string local_sdp_;
        std::mutex ice_mtx_;
        std::vector<CachedIce> cached_ices_;
        std::atomic_bool already_set_answer_sdp_ = false;
    };

}

#endif //GAMMARAY_RTC_CONNECTION_H

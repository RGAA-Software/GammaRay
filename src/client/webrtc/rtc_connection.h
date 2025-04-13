//
// Created by RGAA on 13/04/2025.
//

#ifndef GAMMARAY_RTC_CONNECTION_H
#define GAMMARAY_RTC_CONNECTION_H

#include "rtc_client_interface.h"
#include "tc_common_new/webrtc_helper.h"
#include <memory>

extern "C" __declspec(dllexport) void* GetInstance();

namespace tc
{

    class PeerCallback;
    class SetSessCallback;
    class CreateSessCallback;

    class RtcConnection : public RtcClientInterface {
    public:
        RtcConnection();
        bool Init() override;

    private:
        void CreatePeerConnection();
        void CreatePeerConnectionFactory();

    private:
        std::shared_ptr<PeerCallback> peer_callback_ = nullptr;
        rtc::scoped_refptr<SetSessCallback> set_sess_callback_ = nullptr;
        rtc::scoped_refptr<CreateSessCallback> create_sess_callback_ = nullptr;

        std::unique_ptr<rtc::Thread> network_thread_;
        std::unique_ptr<rtc::Thread> worker_thread_;
        std::unique_ptr<rtc::Thread> sig_thread_;

        rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_conn_ = nullptr;
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_conn_factory_;
        webrtc::PeerConnectionInterface::RTCConfiguration configuration_;

    };

}

#endif //GAMMARAY_RTC_CONNECTION_H

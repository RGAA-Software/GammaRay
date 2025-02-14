//
// Created by hy on 2024/4/25.
//

#ifndef TEST_WEBRTC_RTCSERVER_H
#define TEST_WEBRTC_RTCSERVER_H

#include "webrtc_helper.h"
#include "video_source_mock.h"

namespace tc
{

    class PeerCallback;
    class CreateSessCallback;
    class SetSessCallback;
    class SigManager;
    class RtcContext;
    class DesktopCapture;

    class RtcServer : public std::enable_shared_from_this<RtcServer> {
    public:

        static std::shared_ptr<RtcServer> Make(const std::shared_ptr<RtcContext>& ctx);
        explicit RtcServer(const std::shared_ptr<RtcContext>& ctx);

        bool Start();
        void Exit();

        // callbacks
        void OnSessionCreated(webrtc::SessionDescriptionInterface *desc);
        void OnIceCandidate(const webrtc::IceCandidateInterface *candidate);
        void OnIceGatheringComplete();
        void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver);

    private:
        void CreatePeerConnectionFactory();
        void CreatePeerConnection();

    private:
        std::unique_ptr<rtc::Thread> network_thread_;
        std::unique_ptr<rtc::Thread> worker_thread_;
        std::unique_ptr<rtc::Thread> sig_thread_;
        std::string sdp_;
        std::shared_ptr<PeerCallback> peer_callback_ = nullptr;
        rtc::scoped_refptr<SetSessCallback> set_sess_callback_ = nullptr;
        rtc::scoped_refptr<CreateSessCallback> create_sess_callback_ = nullptr;

        rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_conn_ = nullptr;
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_conn_factory_;
        webrtc::PeerConnectionInterface::RTCConfiguration configuration_;

        rtc::scoped_refptr<VideoTrack> video_track_;
//        VideoTrack* video_track_ = nullptr;
        std::shared_ptr<RtcContext> rtc_ctx_ = nullptr;
        std::shared_ptr<DesktopCapture> desktop_capture_ = nullptr;
        std::shared_ptr<VideoSourceMock> mock_video_source_ = nullptr;
    };

}

#endif //TEST_WEBRTC_RTCSERVER_H

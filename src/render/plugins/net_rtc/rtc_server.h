//
// Created by hy on 2024/4/25.
//

#ifndef TEST_WEBRTC_RTCSERVER_H
#define TEST_WEBRTC_RTCSERVER_H

#include "tc_common_new/webrtc_helper.h"
#include "video_source_mock.h"

namespace tc
{

    class PeerCallback;
    class CreateSessCallback;
    class SetSessCallback;
    class DesktopCapture;
    class RtcPlugin;
    class RtcDataChannel;

    class RtcServer : public std::enable_shared_from_this<RtcServer> {
    public:

        static std::shared_ptr<RtcServer> Make(RtcPlugin* plugin);
        explicit RtcServer(RtcPlugin* plugin);
        RtcPlugin* GetPlugin();

        bool Start(const std::string& stream_id, const std::string& offer_sdp);
        void Exit();
        void OnRemoteIce(const std::string& ice, const std::string& mid, int sdp_mline_index);
        bool IsDataChannelConnected();

        void PostProtoMessage(const std::string &msg, bool run_through = false);
        bool PostTargetStreamProtoMessage(const std::string &stream_id, const std::string &msg, bool run_through = false);
        bool PostTargetFileTransferProtoMessage(const std::string &stream_id, const std::string &msg, bool run_through = false);

        uint32_t GetMediaPendingMessages();
        uint32_t GetFtPendingMessages();

    private:
        void CreatePeerConnectionFactory();
        void CreatePeerConnection();

        void SendSdpToRemote(const std::string& sdp);
        void SendIceToRemote(const std::string& ice, const std::string& mid, int sdp_mline_index);

    private:
        RtcPlugin* plugin_ = nullptr;
        std::unique_ptr<rtc::Thread> network_thread_;
        std::unique_ptr<rtc::Thread> worker_thread_;
        std::unique_ptr<rtc::Thread> sig_thread_;
        std::string stream_id_;
        std::string offer_sdp_;
        std::string sdp_;
        std::shared_ptr<PeerCallback> peer_callback_ = nullptr;
        rtc::scoped_refptr<SetSessCallback> set_remote_offer_sdp_callback_ = nullptr;
        rtc::scoped_refptr<SetSessCallback> set_local_answer_sdp_callback_ = nullptr;
        rtc::scoped_refptr<CreateSessCallback> create_answer_callback_ = nullptr;

        rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_conn_ = nullptr;
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_conn_factory_;
        webrtc::PeerConnectionInterface::RTCConfiguration configuration_;

        std::shared_ptr<RtcDataChannel> media_data_channel_ = nullptr;
        std::shared_ptr<RtcDataChannel> ft_data_channel_ = nullptr;
    };

}

#endif //TEST_WEBRTC_RTCSERVER_H

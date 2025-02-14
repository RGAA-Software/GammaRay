//
// Created by hy on 2024/2/18.
//

#ifndef PEER_CALLBACK_H
#define PEER_CALLBACK_H

#include "webrtc_helper.h"
#include "http/httplib.h"
#include "json/json.hpp"
#include "video_source_mock.h"

using namespace httplib;
using namespace nlohmann;

namespace tc
{

    class RtcServer;

    class PeerCallback : public webrtc::PeerConnectionObserver {
    public:

        static std::shared_ptr<PeerCallback> Make(const std::shared_ptr<RtcServer>& client);

        explicit PeerCallback(const std::shared_ptr<RtcServer>& client);

        // PeerConnection overrides
        void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
        void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
        void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
        void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;
        void OnRenegotiationNeeded() override;
        void OnNegotiationNeededEvent(uint32_t event_id) override;
        void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
        void OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
        void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;
        void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
        void OnIceCandidate(const webrtc::IceCandidateInterface *candidate) override;
        void OnIceCandidateError(const std::string &address, int port, const std::string &url, int error_code,
                                 const std::string &error_text) override;
        void OnIceCandidatesRemoved(const std::vector<cricket::Candidate> &candidates) override;
        void OnIceConnectionReceivingChange(bool receiving) override;
        void OnIceSelectedCandidatePairChanged(const cricket::CandidatePairChangeEvent &event) override;
        void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>> &streams) override;
        void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;
        void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
        void OnInterestingUsage(int usage_pattern) override;

    private:

        std::shared_ptr<RtcServer> rtc_server_ = nullptr;
        std::shared_ptr<VideoStreamReceiver> video_receiver_;
    };

    class CreateSessCallback : public webrtc::CreateSessionDescriptionObserver {
    public:
        static rtc::scoped_refptr<CreateSessCallback> Make(const std::shared_ptr<RtcServer>& srv);
        explicit CreateSessCallback(const std::shared_ptr<RtcServer>& srv);
        void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;
        void OnFailure(webrtc::RTCError error) override;

    private:
        std::shared_ptr<RtcServer> srv_server_ = nullptr;
    };

    class SetSessCallback : public webrtc::SetSessionDescriptionObserver {
    public:
        static rtc::scoped_refptr<SetSessCallback> Make(const std::shared_ptr<RtcServer>& srv);
        explicit SetSessCallback(const std::shared_ptr<RtcServer>& srv);
        void OnSuccess() override;
        void OnFailure(webrtc::RTCError error) override;

    private:
        std::shared_ptr<RtcServer> rtc_server_ = nullptr;

    };

}

#endif //PEER_CALLBACK_H

//
// Created by RGAA on 6/08/2024.
//

#ifndef GAMMARAY_RTC_PEER_CALLBACK_H
#define GAMMARAY_RTC_PEER_CALLBACK_H

#include "rtc_header.h"

namespace tc
{

    class RtcPeerCallback : webrtc::PeerConnectionObserver {
    public:
    private:
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
        void OnIceCandidateError(const std::string &address, int port, const std::string &url, int error_code, const std::string &error_text) override;
        void OnIceCandidatesRemoved(const std::vector<cricket::Candidate> &candidates) override;
        void OnIceConnectionReceivingChange(bool receiving) override;
        void OnIceSelectedCandidatePairChanged(const cricket::CandidatePairChangeEvent &event) override;
        void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>> &streams) override;
        void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;
        void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
        void OnInterestingUsage(int usage_pattern) override;
    };

}

#endif //GAMMARAY_RTC_PEER_CALLBACK_H

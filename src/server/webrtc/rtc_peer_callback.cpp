//
// Created by RGAA on 6/08/2024.
//

#include "rtc_peer_callback.h"

namespace tc
{

    void tc::RtcPeerCallback::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {

    }

    void tc::RtcPeerCallback::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
        PeerConnectionObserver::OnAddStream(stream);
    }

    void tc::RtcPeerCallback::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
        PeerConnectionObserver::OnRemoveStream(stream);
    }

    void tc::RtcPeerCallback::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {

    }

    void tc::RtcPeerCallback::OnRenegotiationNeeded() {
        PeerConnectionObserver::OnRenegotiationNeeded();
    }

    void tc::RtcPeerCallback::OnNegotiationNeededEvent(uint32_t event_id) {
        PeerConnectionObserver::OnNegotiationNeededEvent(event_id);
    }

    void tc::RtcPeerCallback::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
        PeerConnectionObserver::OnIceConnectionChange(new_state);
    }

    void
    tc::RtcPeerCallback::OnStandardizedIceConnectionChange(
            webrtc::PeerConnectionInterface::IceConnectionState new_state) {
        PeerConnectionObserver::OnStandardizedIceConnectionChange(new_state);
    }

    void tc::RtcPeerCallback::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) {
        PeerConnectionObserver::OnConnectionChange(new_state);
    }

    void tc::RtcPeerCallback::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {

    }

    void tc::RtcPeerCallback::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {

    }

    void tc::RtcPeerCallback::OnIceCandidateError(const std::string &address, int port, const std::string &url, int error_code, const std::string &error_text) {
        PeerConnectionObserver::OnIceCandidateError(address, port, url, error_code, error_text);
    }

    void tc::RtcPeerCallback::OnIceCandidatesRemoved(const std::vector<cricket::Candidate> &candidates) {
        PeerConnectionObserver::OnIceCandidatesRemoved(candidates);
    }

    void tc::RtcPeerCallback::OnIceConnectionReceivingChange(bool receiving) {
        PeerConnectionObserver::OnIceConnectionReceivingChange(receiving);
    }

    void tc::RtcPeerCallback::OnIceSelectedCandidatePairChanged(const cricket::CandidatePairChangeEvent &event) {
        PeerConnectionObserver::OnIceSelectedCandidatePairChanged(event);
    }

    void tc::RtcPeerCallback::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                                         const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>> &streams) {
        PeerConnectionObserver::OnAddTrack(receiver, streams);
    }

    void tc::RtcPeerCallback::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
        PeerConnectionObserver::OnTrack(transceiver);
    }

    void tc::RtcPeerCallback::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
        PeerConnectionObserver::OnRemoveTrack(receiver);
    }

    void tc::RtcPeerCallback::OnInterestingUsage(int usage_pattern) {
        PeerConnectionObserver::OnInterestingUsage(usage_pattern);
    }

}
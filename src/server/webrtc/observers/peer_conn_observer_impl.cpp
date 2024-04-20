//
// Created by RGAA on 2024/2/18.
//

#include "peer_conn_observer_impl.h"
#include "webrtc_server_impl.h"

#include <iostream>
#include <thread>

namespace tc
{

    std::shared_ptr<PeerConnObserverImpl> PeerConnObserverImpl::Make(const std::shared_ptr<WebRtcServerImpl>& client) {
        return std::make_shared<PeerConnObserverImpl>(client);
    }

    PeerConnObserverImpl::PeerConnObserverImpl(const std::shared_ptr<WebRtcServerImpl>& client) {
        webrtc_client_ = client;
    }

    void PeerConnObserverImpl::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
        std::cout << std::this_thread::get_id() << ":"
                  << "PeerConnectionObserver::SignalingChange(" << new_state << ")" << std::endl;
    }

    void PeerConnObserverImpl::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
        PeerConnectionObserver::OnAddStream(stream);
        std::cout << std::this_thread::get_id() << ":"
                  << "PeerConnectionObserver::AddStream" << ", video track size: " << stream->GetVideoTracks().size() << std::endl;
    }

    void PeerConnObserverImpl::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
        PeerConnectionObserver::OnRemoveStream(stream);
        std::cout << std::this_thread::get_id() << ":"
                  << "PeerConnectionObserver::RemoveStream" << std::endl;
    }

    void PeerConnObserverImpl::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {

    }

    void PeerConnObserverImpl::OnRenegotiationNeeded() {
        PeerConnectionObserver::OnRenegotiationNeeded();
        std::cout << std::this_thread::get_id() << ":"
                  << "PeerConnectionObserver::RenegotiationNeeded" << std::endl;
    }

    void PeerConnObserverImpl::OnNegotiationNeededEvent(uint32_t event_id) {
        PeerConnectionObserver::OnNegotiationNeededEvent(event_id);
    }

    void PeerConnObserverImpl::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
        PeerConnectionObserver::OnIceConnectionChange(new_state);
        std::cout << std::this_thread::get_id() << ":"
                  << "PeerConnectionObserver::IceConnectionChange(" << new_state << ")" << std::endl;
    }

    void PeerConnObserverImpl::OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
        PeerConnectionObserver::OnStandardizedIceConnectionChange(new_state);
    }

    void PeerConnObserverImpl::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) {
        PeerConnectionObserver::OnConnectionChange(new_state);
    }

    void PeerConnObserverImpl::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
        std::cout << std::this_thread::get_id() << ":"
                  << "PeerConnectionObserver::IceGatheringChange(" << new_state << ")" << std::endl;
        if (new_state == webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete) {
            webrtc_client_->OnIceGatheringComplete();
        }
    }

    void PeerConnObserverImpl::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
        std::cout << ":" << std::this_thread::get_id() << ":"
                  << "PeerConnectionObserver::IceCandidate" << std::endl;
        webrtc_client_->OnIceCandidate(candidate);
    }

    void PeerConnObserverImpl::OnIceCandidateError(const std::string &address, int port, const std::string &url,
                                                   int error_code, const std::string &error_text) {
        PeerConnectionObserver::OnIceCandidateError(address, port, url, error_code, error_text);
    }

    void PeerConnObserverImpl::OnIceCandidatesRemoved(const std::vector<cricket::Candidate> &candidates) {
        PeerConnectionObserver::OnIceCandidatesRemoved(candidates);
    }

    void PeerConnObserverImpl::OnIceConnectionReceivingChange(bool receiving) {
        PeerConnectionObserver::OnIceConnectionReceivingChange(receiving);
    }

    void PeerConnObserverImpl::OnIceSelectedCandidatePairChanged(const cricket::CandidatePairChangeEvent &event) {
        PeerConnectionObserver::OnIceSelectedCandidatePairChanged(event);
    }

    void PeerConnObserverImpl::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                                          const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>> &streams) {
        PeerConnectionObserver::OnAddTrack(receiver, streams);
        std::cout << "OnAddTrack..." << std::endl;
    }

    void PeerConnObserverImpl::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
        PeerConnectionObserver::OnTrack(transceiver);
        webrtc_client_->OnTrack(transceiver);
    }

    void PeerConnObserverImpl::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
        PeerConnectionObserver::OnRemoveTrack(receiver);
    }

    void PeerConnObserverImpl::OnInterestingUsage(int usage_pattern) {
        PeerConnectionObserver::OnInterestingUsage(usage_pattern);
    }
}
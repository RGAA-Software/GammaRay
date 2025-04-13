//
// Created by hy on 2024/2/18.
//

#include "peer_callback.h"
#include "tc_common_new/log.h"
#include <iostream>
#include <thread>

namespace tc
{

    std::shared_ptr<PeerCallback> PeerCallback::Make(RtcConnection* client) {
        return std::make_shared<PeerCallback>(client);
    }

    PeerCallback::PeerCallback(RtcConnection* client) {

    }

    void PeerCallback::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
        LOGI("PeerCallback::SignalingChange: {}", (int)new_state);
    }

    void PeerCallback::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
        PeerConnectionObserver::OnAddStream(stream);
        LOGI("OnAddStream, stream id: {}", stream->id());
    }

    void PeerCallback::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
        PeerConnectionObserver::OnRemoveStream(stream);
        LOGI("OnRemoveStream");
    }

    void PeerCallback::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {

    }

    void PeerCallback::OnRenegotiationNeeded() {
        PeerConnectionObserver::OnRenegotiationNeeded();
    }

    void PeerCallback::OnNegotiationNeededEvent(uint32_t event_id) {
        PeerConnectionObserver::OnNegotiationNeededEvent(event_id);
    }

    void PeerCallback::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
        PeerConnectionObserver::OnIceConnectionChange(new_state);
        if (new_state == webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionConnected) {
            LOGI("ICE -- Connected.");
        }
        LOGI("OnIceConnectionChange ==> state: {}", (int)new_state);
    }

    void PeerCallback::OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
        PeerConnectionObserver::OnStandardizedIceConnectionChange(new_state);
    }

    void PeerCallback::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) {
        PeerConnectionObserver::OnConnectionChange(new_state);
    }

    void PeerCallback::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
        LOGI("PeerCallback::IceGatheringChange: {}", (int)new_state);
        if (new_state == webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete) {
            //rtc_server_->OnIceGatheringComplete();
        }
    }

    void PeerCallback::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
        LOGI("PeerCallback::IceCandidate");
        //rtc_server_->OnIceCandidate(candidate);
    }

    void PeerCallback::OnIceCandidateError(const std::string &address, int port, const std::string &url,
                                                   int error_code, const std::string &error_text) {
        PeerConnectionObserver::OnIceCandidateError(address, port, url, error_code, error_text);
    }

    void PeerCallback::OnIceCandidatesRemoved(const std::vector<cricket::Candidate> &candidates) {
        PeerConnectionObserver::OnIceCandidatesRemoved(candidates);
    }

    void PeerCallback::OnIceConnectionReceivingChange(bool receiving) {
        PeerConnectionObserver::OnIceConnectionReceivingChange(receiving);
    }

    void PeerCallback::OnIceSelectedCandidatePairChanged(const cricket::CandidatePairChangeEvent &event) {
        PeerConnectionObserver::OnIceSelectedCandidatePairChanged(event);
    }

    void PeerCallback::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                                          const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>> &streams) {
        PeerConnectionObserver::OnAddTrack(receiver, streams);
        LOGI("OnAddTrack");
        auto track = receiver->track().get();
        LOGI("[info] on add track,kind:{}", track->kind());
//        if(track->kind() == "video" && video_receiver_) {
//            auto cast_track = static_cast<webrtc::VideoTrackInterface*>(track);
//            cast_track->AddOrUpdateSink(video_receiver_.get(), rtc::VideoSinkWants());
//        }
    }

    void PeerCallback::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
        PeerConnectionObserver::OnTrack(transceiver);
//        rtc_server_->OnTrack(transceiver);
    }

    void PeerCallback::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
        PeerConnectionObserver::OnRemoveTrack(receiver);
    }

    void PeerCallback::OnInterestingUsage(int usage_pattern) {
        PeerConnectionObserver::OnInterestingUsage(usage_pattern);
    }

    ///
    rtc::scoped_refptr<CreateSessCallback> CreateSessCallback::Make(RtcConnection* srv) {
        auto r =  new rtc::RefCountedObject<CreateSessCallback>(srv);
        return rtc::scoped_refptr<rtc::RefCountedObject<CreateSessCallback>>(r);
    }

    CreateSessCallback::CreateSessCallback(RtcConnection* srv) {
//        this->srv_server_ = srv;
    }

    void CreateSessCallback::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
        LOGI("@@ CreateSessCallback::OnSuccess");
//        this->srv_server_->OnSessionCreated(desc);
    }

    void CreateSessCallback::OnFailure(webrtc::RTCError error) {
        LOGE("@@ CreateSessCallback::OnFailure: {}", error.message());
    }

    ///
    rtc::scoped_refptr<SetSessCallback> SetSessCallback::Make(RtcConnection* srv) {
        auto c = new rtc::RefCountedObject<SetSessCallback>(srv);
        return rtc::scoped_refptr<rtc::RefCountedObject<SetSessCallback>>(c);
    }

    SetSessCallback::SetSessCallback(RtcConnection* srv) {
//        this->rtc_server_ = srv;
    }

    void SetSessCallback::OnSuccess() {
        LOGI("@@ SetSessCallback::OnSuccess");
    }

    void SetSessCallback::OnFailure(webrtc::RTCError error) {
        LOGE("@@ SetSessCallback::OnFailure: {}", error.message());
    }

}
//
// Created by hy on 2024/2/18.
//

#ifndef SDK_PEER_CALLBACK_H
#define SDK_PEER_CALLBACK_H

#include "tc_common_new/webrtc_helper.h"
#include "http/httplib.h"
#include "json/json.hpp"

using namespace httplib;
using namespace nlohmann;

namespace tc
{

    using OnCreateSdpSuccessCallback = std::function<void(webrtc::SessionDescriptionInterface*)>;
    using OnCreateSdpFailedCallback = std::function<void(const std::string&)>;
    using OnSetSdpSuccessCallback = std::function<void()>;
    using OnSetSdpFailedCallback = std::function<void(const std::string&)>;
    using OnIceCallback = std::function<void(const std::string& ice, const std::string& mid, int sdp_mline_index)>;

    class RtcConnection;

    class PeerCallback : public webrtc::PeerConnectionObserver {
    public:

        static std::shared_ptr<PeerCallback> Make(RtcConnection* client);

        explicit PeerCallback(RtcConnection* client);

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

        //
        void SetOnIceCallback(OnIceCallback&& cbk) {
            ice_callback_ = cbk;
        }

    private:
        OnIceCallback ice_callback_;
    };

    class CreateSessCallback : public webrtc::CreateSessionDescriptionObserver {
    public:
        static rtc::scoped_refptr<CreateSessCallback> Make(RtcConnection* srv);
        explicit CreateSessCallback(RtcConnection* srv);
        void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;
        void OnFailure(webrtc::RTCError error) override;

        void SetOnCreateSdpSuccessCallback(OnCreateSdpSuccessCallback&& cbk) {
            cbk_success_ = cbk;
        }

        void SetOnCreateSdpFailedCallback(OnCreateSdpFailedCallback&& cbk) {
            cbk_failed_ = cbk;
        }

    private:
        OnCreateSdpSuccessCallback cbk_success_;
        OnCreateSdpFailedCallback cbk_failed_;
    };

    class SetSessCallback : public webrtc::SetSessionDescriptionObserver {
    public:
        static rtc::scoped_refptr<SetSessCallback> Make(RtcConnection* srv);
        explicit SetSessCallback(RtcConnection* srv);
        void OnSuccess() override;
        void OnFailure(webrtc::RTCError error) override;

        void SetSdpSuccessCallback(OnSetSdpSuccessCallback&& cbk) {
            cbk_success_ = cbk;
        }

        void SetSdpFailedCallback(OnSetSdpFailedCallback&& cbk) {
            cbk_failed_ = cbk;
        }

    private:
        OnSetSdpSuccessCallback cbk_success_;
        OnSetSdpFailedCallback cbk_failed_;
    };

}

#endif //PEER_CALLBACK_H

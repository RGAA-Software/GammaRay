//
// Created by RGAA on 2024/3/4.
//

#ifndef TEST_WEBRTC_SIGNALING_MANAGER_H
#define TEST_WEBRTC_SIGNALING_MANAGER_H

#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <asio2/asio2.hpp>

namespace dl
{

    class SignalingMessage {
    public:
        std::string room_id_;
        std::string session_id_;
        std::string signal_;
        std::string message_;
        std::string sdp_mid_;
        int sdp_mline_index_{};
    };

    class SignalingParam {
    public:
        std::string host_{};
        int port_{};
        std::string path_{};
    };

    using OnSignalingCallback = std::function<void(std::shared_ptr<SignalingMessage>&&)>;

    class SignalingManager {
    public:

        static std::shared_ptr<SignalingManager> Make();

        SignalingManager();
        ~SignalingManager();
        void Start(const SignalingParam& param);

        void Join(const std::string& roomId, const std::string& sessionId);
        void Leave(const std::string& roomId, const std::string& sessionId);
        void SendSdp(const std::string& roomId, const std::string& sessionId, const std::string& sdp);
        void SendIceCandidate(const std::string& roomId, const std::string& sessionId, const std::string& ice,
                              const std::string& sdp_mid, int sdp_mline_index);

        void RegisterJoinedCallback(OnSignalingCallback&& cbk) {on_joined_cbk_ = cbk;}
        void RegisterOtherJoinedCallback(OnSignalingCallback&& cbk) {on_other_joined_cbk_ = cbk;}
        void RegisterLeaveCallback(OnSignalingCallback&& cbk) {on_leave_cbk_ = cbk;}
        void RegisterOtherLeaveCallback(OnSignalingCallback&& cbk) {on_other_leave_cbk_ = cbk;}
        void RegisterSdpCallback(OnSignalingCallback&& cbk) {on_sdp_cbk_ = cbk;}
        void RegisterIceCandidateCallback(OnSignalingCallback&& cbk) {on_ice_candidate_cbk_ = cbk;}

    private:
        std::shared_ptr<SignalingMessage> ParseSignalingMessage(std::string_view data);
        void DispatchSignalingMessage(std::shared_ptr<SignalingMessage>&& msg);

    private:
        SignalingParam sig_param_;
        std::shared_ptr<asio2::ws_client> client_ = nullptr;

        OnSignalingCallback on_joined_cbk_;
        OnSignalingCallback on_other_joined_cbk_;
        OnSignalingCallback on_leave_cbk_;
        OnSignalingCallback on_other_leave_cbk_;
        OnSignalingCallback on_sdp_cbk_;
        OnSignalingCallback on_ice_candidate_cbk_;
    };

}

#endif //TEST_WEBRTC_SIGNALING_MANAGER_H

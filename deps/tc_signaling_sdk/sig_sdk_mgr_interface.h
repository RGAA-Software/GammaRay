#pragma once

#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <asio2/asio2.hpp>
#include "sig_sdk_message.h"
#include "json/json.hpp"
#include "sig_sdk_params.h"

namespace tc
{

    class RtcContext;
    class MessageNotifier;
    class SigRouterInterface;

    class SigMgrInterface {
    public:
        explicit SigMgrInterface(const std::shared_ptr<RtcContext>& ctx);

        virtual void Start(const SignalingParam& param);
        virtual void Stop();
        virtual bool IsAlive();

        virtual void SendHello(const SigHelloMessage& msg);
        virtual void CreateRoom(const SigCreateRoomMessage& msg);
        virtual void JoinRoom(const SigJoinRoomMessage& msg);
        virtual void LeaveRoom(const SigLeaveRoomMessage& msg);
        virtual void InviteClient(const SigInviteClientMessage& msg);
        virtual void ReqRemoteInfo(const SigReqRemoteInfoMessage& msg);
        virtual void OnRemoteInfo(const SigOnRemoteInfoMessage& msg);
        virtual void SendOfferSdp(const SigOfferSdpMessage& msg);
        virtual void SendAnswerSdp(const SigAnswerSdpMessage& msg);
        virtual void SendIceCandidate(const SigIceMessage& msg);
        virtual void SendDataChannelReady(const SigOnDataChannelReadyMessage& msg);
        virtual void SendReqControl(const SigReqControlMessage& msg);
        virtual void SendUnderControl(const SigUnderControlMessage& msg);
        virtual void ParseSigMessage(const std::string& msg);
        virtual void SendSigMessage(const std::string& sig_name, const std::string& token, const std::string& msg);

    private:
        std::vector<std::shared_ptr<Client>> ParseClients(const nlohmann::json& obj);
        std::vector<std::string> ParseIps(const nlohmann::json& obj);

    protected:
        SignalingParam sig_param_;
        std::shared_ptr<RtcContext> rtc_ctx_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        int64_t heart_beat_idx_ = 0;
        std::shared_ptr<SigRouterInterface> sig_router_ = nullptr;
    };

}
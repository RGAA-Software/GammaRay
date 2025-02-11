//
// Created by RGAA
//

#ifndef TC_WEBRTC_SIG_MAKER_H
#define TC_WEBRTC_SIG_MAKER_H

#include "json/json.hpp"
#include "sig_sdk_message.h"

namespace tc
{

    class SigSdkMessageMaker {
    public:
        static std::string MakeOfferSdp(const SigOfferSdpMessage& msg);
        static std::string MakeAnswerSdp(const SigAnswerSdpMessage& msg);
        static std::string MakeIceCandidate(const SigIceMessage& msg);
        static std::string MakeHello(const SigHelloMessage& msg);
        static std::string MakeCreateRoom(const SigCreateRoomMessage& msg);
        static std::string MakeJoinRoom(const SigJoinRoomMessage& room);
        static std::string MakeInviteRemoteClient(const SigInviteClientMessage& msg);
        static std::string MakeLeaveRoom(const SigLeaveRoomMessage& msg);
        static std::string MakeReqRemoteInfo(const SigReqRemoteInfoMessage& msg);
        static std::string MakeOnRemoteInfo(const SigOnRemoteInfoMessage& msg);
        static std::string MakeHeartBeat(const SigHeartBeatMessage& msg);
        static std::string MakeDataChannelReady(const SigOnDataChannelReadyMessage& msg);
        static std::string MakeReqControl(const SigReqControlMessage& msg);
        static std::string MakeUnderControl(const SigUnderControlMessage& msg);
        static std::string MakeOnRejectControl(const SigOnRejectControlMessage& msg);
    };

}

#endif //TC_WEBRTC_SIG_MAKER_H

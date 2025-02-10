//
// Created by RGAA on 2024/3/5.
//

#include "sig_sdk_maker.h"
#include "tc_common_new/log.h"

using namespace nlohmann;

namespace tc
{

    std::string SigMaker::MakeOfferSdp(const SigOfferSdpMessage& msg) {
        json obj;
        obj[kKeySigName] = kSigOfferSdp;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyRoomId] = msg.room_id_;
        obj[kKeySdp] = msg.sdp_;
        obj[kKeyReqVideoTrackIndex] = msg.request_video_track_index;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        return obj.dump();
    }

    std::string SigMaker::MakeAnswerSdp(const SigAnswerSdpMessage& msg) {
        json obj;
        obj[kKeySigName] = kSigAnswerSdp;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyRoomId] = msg.room_id_;
        obj[kKeySdp] = msg.sdp_;
        obj[kKeyPeerConnId] = msg.peer_conn_id;
        obj[kKeyReqVideoTrackIndex] = msg.video_track_index;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        return obj.dump();
    }

    std::string SigMaker::MakeIceCandidate(const SigIceMessage& msg) {
        json obj;
        obj[kKeySigName] = kSigIce;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyRoomId] = msg.room_id_;
        obj[kKeyIce] = msg.ice_;
        obj[kKeyMid] = msg.mid_;
        obj[kKeySdpMLineIndex] = msg.sdp_mline_index_;
        obj[kKeyPeerConnId] = msg.peer_conn_id;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        return obj.dump();
    }

    std::string SigMaker::MakeHello(const SigHelloMessage& msg) {
        json obj;
        obj[kKeySigName] = kSigHello;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyPlatform] = msg.platform_;
        obj[kKeyAllowResend] = msg.allow_resend_;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        // local ips
        auto local_ip_array = json::array();
        for (const auto& ip : msg.local_ips_) {
            local_ip_array.push_back(ip);
        }
        obj[kKeyLocalIps] = local_ip_array;

        // www ips
        auto www_ip_array = json::array();
        for (const auto& ip : msg.www_ips_) {
            www_ip_array.push_back(ip);
        }
        obj[kKeyWwwIps] = www_ip_array;

        LOGI("make sig: {}, client id: {}, platform: {}", obj[kKeySigName].get<std::string>(),
            obj[kKeyClientId].get<std::string>(),
            obj[kKeyPlatform].get<std::string>());
        return obj.dump();
    }

    std::string SigMaker::MakeCreateRoom(const SigCreateRoomMessage& msg) {
        json obj;
        obj[kKeySigName] = kSigCreateRoom;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyRemoteClientId] = msg.remote_client_id_;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        return obj.dump();
    }

    std::string SigMaker::MakeJoinRoom(const SigJoinRoomMessage& msg) {
        json obj;
        obj[kKeySigName] = kSigJoinRoom;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyRemoteClientId] = msg.remote_client_id_;
        obj[kKeyRoomId] = msg.room_id_;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        return obj.dump();
    }

    std::string SigMaker::MakeLeaveRoom(const SigLeaveRoomMessage& msg) {
        json obj;
        obj[kKeySigName] = kSigLeaveRoom;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyRoomId] = msg.room_id_;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        return obj.dump();
    }

    std::string SigMaker::MakeReqRemoteInfo(const SigReqRemoteInfoMessage& msg) {
        json obj;
        obj[kKeySigName] = kSigReqRemoteInfo;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyRemoteClientId] = msg.remote_client_id_;
        return obj.dump();
    }

    std::string SigMaker::MakeOnRemoteInfo(const SigOnRemoteInfoMessage& msg) {
        json obj;
        obj[kKeySigName] = kSigOnRemoteInfo;
        obj[kKeyToken] = msg.token_;
        obj[kKeySelfClientId] = msg.self_client_id_;
        obj[kKeyControllerClientId] = msg.controller_client_id_;

        auto local_ips_array = json::array();
        for (const auto& ip : msg.local_ips_) {
            local_ips_array.push_back(ip);
        }
        obj[kKeyLocalIps] = local_ips_array;

        auto www_ips_array = json::array();
        for (const auto& ip : msg.www_ips_) {
            www_ips_array.push_back(ip);
        }
        obj[kKeyWwwIps] = www_ips_array;

        obj[kKeyWwwIpTime] = msg.www_ip_time_;

        return obj.dump();
    }

    std::string SigMaker::MakeInviteRemoteClient(const SigInviteClientMessage& msg) {
        json obj;
        obj[kKeySigName] = kSigInviteClient;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyRemoteClientId] = msg.remote_client_id_;
        obj[kKeyRoomId] = msg.room_id_;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        return obj.dump();
    }

    std::string SigMaker::MakeHeartBeat(const SigHeartBeatMessage& msg) {
        json obj;
        obj[kKeySigName] = kSigHeartBeat;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyIndex] = msg.index_;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        // local ips
        auto local_ip_array = json::array();
        for (const auto& ip : msg.local_ips_) {
            local_ip_array.push_back(ip);
        }
        obj[kKeyLocalIps] = local_ip_array;

        // www ips
        auto www_ip_array = json::array();
        for (const auto& ip : msg.www_ips_) {
            www_ip_array.push_back(ip);
        }
        obj[kKeyWwwIps] = www_ip_array;

        return obj.dump();
    }

    std::string SigMaker::MakeDataChannelReady(const SigOnDataChannelReadyMessage& msg) {
        json obj;
        obj[kKeySigName] = msg.sig_name_;
        obj[kKeyToken] = msg.token_;
        obj[kKeySelfClientId] = msg.self_client_id_;
        obj[kKeyControllerClientId] = msg.controller_client_id_;
        obj[kKeyRoomId] = msg.room_id_;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        return obj.dump();
    }

    std::string SigMaker::MakeReqControl(const SigReqControlMessage& msg) {
        json obj;
        obj[kKeySigName] = msg.sig_name_;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyRemoteClientId] = msg.remote_client_id_;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        return obj.dump();
    }

    std::string SigMaker::MakeUnderControl(const SigUnderControlMessage& msg) {
        json obj;
        obj[kKeySigName] = msg.sig_name_;
        obj[kKeyToken] = msg.token_;
        obj[kKeySelfClientId] = msg.self_client_id_;
        obj[kKeyControllerClientId] = msg.controller_client_id_;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        return obj.dump();
    }

    std::string SigMaker::MakeOnRejectControl(const SigOnRejectControlMessage& msg) {
        json obj;
        obj[kKeySigName] = msg.sig_name_;
        obj[kKeyToken] = msg.token_;
        obj[kKeyClientId] = msg.client_id_;
        obj[kKeyControllerClientId] = msg.controller_client_id_;
        obj[kKeyRoomId] = msg.room_id_;
        obj[kKeyGroupId] = msg.group_id_;
        obj[kKeyUserId] = msg.user_id_;
        return obj.dump();
    }

}

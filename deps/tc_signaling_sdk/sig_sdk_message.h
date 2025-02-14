//
// Created by RGAA
//

#ifndef TEST_WEBRTC_SIG_MESSAGE_H
#define TEST_WEBRTC_SIG_MESSAGE_H

#include <string>
#include <cstdint>
#include <memory>
#include <sstream>
#include <vector>

#include "tc_common_new/concurrent_vector.h"
#include "json/json.hpp"
#include "sig_sdk_context.h"

namespace tc
{

    const std::string kSigHello = "hello";
    const std::string kSigOnHello = "on_hello";
    const std::string kSigError = "sig_error";
    const std::string kSigCreateRoom = "create_room";
    const std::string kSigOnCreatedRoom = "on_created_room";
    const std::string kSigJoinRoom = "join_room";
    const std::string kSigOnJoinedRoom = "on_joined_room";
    const std::string kSigOnRemoteJoinedRoom = "on_remote_joined_room";
    const std::string kSigLeaveRoom = "leave_room";
    const std::string kSigOnLeftRoom = "on_left_room";
    const std::string kSigOnRemoteLeftRoom = "on_remote_left_room";
    const std::string kSigInviteClient = "invite_client";
    const std::string kSigOnInvitedToRoom = "on_invited_to_room";
    const std::string kSigOnRemoteInvitedToRoom = "on_remote_invited_to_room";
    const std::string kSigHeartBeat = "heart_beat";
    const std::string kSigOnHeartBeat = "on_heart_beat";
    const std::string kSigOfferSdp = "offer_sdp";
    const std::string kSigAnswerSdp = "answer_sdp";
    const std::string kSigIce = "ice";
    const std::string kSigReqControl = "sig_req_control";
    const std::string kSigUnderControl = "sig_under_control";
    const std::string kSigOnDataChannelReady = "on_data_channel_ready";
    const std::string kSigOnRejectControl = "on_reject_control";
    const std::string kSigReqRemoteInfo = "req_remote_info";
    const std::string kSigOnRemoteInfo = "on_remote_info";
    const std::string kSigSendMessage = "send_message";
    const std::string kSigOnProcessedMessage = "processed_message";

    const std::string kKeySigName = "sig_name";
    const std::string kKeyToken = "token";
    const std::string kKeySigCode= "sig_code";
    const std::string kKeySigInfo = "sig_info";
    const std::string kKeyClientId = "client_id";
    const std::string kKeyRoomId = "room_id";
    const std::string kKeyRemoteClientId = "remote_client_id";
    const std::string kKeyClients = "clients";
    const std::string kKeyInvitorClientId = "invitor_client_id";
    const std::string kKeySelfClientId = "self_client_id";
    const std::string kKeyControllerClientId = "controller_client_id";
    const std::string kKeyIndex = "index";
    const std::string kKeyPlatform = "platform";
    const std::string kKeyId = "id";
    const std::string kKeyRole = "role";
    const std::string kKeyName = "name";
    const std::string kKeyUpdateTimestamp = "update_timestamp";
    const std::string kKeySdp = "sdp";
    const std::string kKeyIce = "ice";
    const std::string kKeyMid = "mid";
    const std::string kKeySdpMLineIndex = "sdp_m_line_index";
    const std::string kKeyPeerConnId = "peer_conn_id";
    const std::string kKeyReqVideoTrackIndex = "req_video_track_index";
    const std::string kKeyAllowResend = "allow_resend";
    const std::string kKeyGroupId = "group_id";
    const std::string kKeyUserId = "user_id";
    const std::string kKeyLocalIps = "local_ips";
    const std::string kKeyWwwIps = "www_ips";
    const std::string kKeyWwwIpTime = "www_ip_time";
    const std::string kKeyMessage = "message";
    const std::string kKeyIceServers = "ice_servers";
    const std::string kKeyIceUsername = "username";
    const std::string kKeyIceUrls = "urls";
    const std::string kKeyIceCredential = "credential";
    const std::string kKeySessionId = "session_id";

    enum SignalingErrorCode {
        kSignalingErrorCodeNoError = 1000,
        kSignalingErrorCodeAuth = 600,
        kSignalingErrorCodeInvalidParam = 601,
        kSignalingErrorCodeNoRoomFound = 602,
        kSignalingErrorCodeCreateRoomFailed = 603,
        kSignalingErrorCodeClientOffline = 604,
        kSignalingErrorCodeNoClientFound = 605,
        kSignalingErrorCodeCommandNotProcessed = 606,
        kSignalingErrorCodeAlreadyLogin = 607,
    };

    // Original
    class SignalingMessage {
    public:
        std::string room_id_;
        std::string session_id_;
        std::string signal_;
        std::string message_;
        std::string sdp_mid_;
        int sdp_mline_index_{};
    };

    // Client
    struct Client {
        std::string id_;
        std::string name_;
        std::string role_;
        std::string platform_;
        std::string room_id_;
        int64_t update_timestamp_;

        std::string Dump() {
            std::stringstream ss;
            ss << "Client id: " << id_ << ", platform: " << platform_ << ", timestamp: " << update_timestamp_ << std::endl;
            return ss.str();
        }

        std::string AsJson() {
            nlohmann::json obj;
            obj["id"] = id_;
            obj["name"] = name_;
            obj["platform"] = platform_;
            obj["room_id"] = room_id_;
            return obj.dump(2);
        }
    };
    using ClientPtr = std::shared_ptr<Client>;

    // Room
    struct Room {
        std::string id_;
        std::string name_;
        std::vector<ClientPtr> clients_;

        std::string Dump() {
            std::stringstream ss;
            ss << "Room id: " << id_ << std::endl;
            ss << "Client size: " << clients_.size() << std::endl;
            for (auto& c : clients_) {
                ss << "--client id: " << c->id_ << std::endl;
                ss << "  client platform: " << c->platform_ << std::endl;
                ss << "  client timestamp: " << c->update_timestamp_ << std::endl;
            }
            return ss.str();
        }
    };
    using RoomPtr = std::shared_ptr<Room>;

    // Messages
    struct SigBaseMessage {
        std::string sig_name_;
        std::string token_;
        std::string sig_origin_msg_;
        std::string group_id_;
        std::string user_id_;
    };
    struct SigErrorMessage : public SigBaseMessage {
        int code_;
        std::string info_;
    };

    // SigHelloMessage hello消息
    // client -> server
    struct SigHelloMessage : public SigBaseMessage {
        std::string client_id_;
        std::string platform_;
        bool allow_resend_;
        std::vector<std::string> local_ips_;
        std::vector<std::string> www_ips_;
    };

    // SigCreateRoomMessage 请求创建一个房间，如果已经存在，则直接返回
    // client -> server
    struct SigCreateRoomMessage : public SigBaseMessage {
        std::string client_id_;
        std::string remote_client_id_;
    };

    // SigOnHelloMessage hello回复
    // server -> client
    struct SigOnHelloMessage : public SigBaseMessage {
        std::string client_id_;
        // 信令服务配置的ice_server,0端口场景从信令服务获取此项配置。
        std::vector<SigSdkIceServer> ice_servers;
    };

    // SigOnCreatedRoomMessage 创建完成回调给发起创建者
    // server -> client
    struct SigOnCreatedRoomMessage : public SigBaseMessage {
        std::string client_id_;
        std::string remote_client_id_;
        std::string room_id_;
    };

    // SigJoinRoomMessage 加入一个房间
    // client -> server
    struct SigJoinRoomMessage : public SigBaseMessage {
        std::string client_id_;
        std::string remote_client_id_;
        std::string room_id_;
    };

    // SigOnJoinedRoomMessage 加入一个房间后，回调给请求加入的人
    // server -> client
    struct SigOnJoinedRoomMessage : public SigBaseMessage {
        std::string client_id_;
        std::string remote_client_id_;
        std::string room_id_;
        std::vector<ClientPtr> clients_;
    };

    // SigOnRemoteJoinedRoomMessage 其他成员加入
    // server -> client
    struct SigOnRemoteJoinedRoomMessage : public SigBaseMessage {
        std::string remote_client_id_;
        std::string room_id_;
        std::vector<ClientPtr> clients_;
    };

    // SigLeaveRoomMessage 请求离开房间
    // client -> server
    struct SigLeaveRoomMessage : public SigBaseMessage {
        std::string client_id_;
        std::string room_id_;
    };

    // SigOnLeftRoomMessage 自己离开房间
    // server -> client
    struct SigOnLeftRoomMessage : public SigBaseMessage {
        std::string client_id_;
        std::string room_id_;
        std::vector<ClientPtr> clients_;
    };

    // SigOnOtherLeftRoomMessage 其他成员离开
    // server -> client
    struct SigOnRemoteLeftRoomMessage : public SigBaseMessage {
        std::string remote_client_id_;
        std::string room_id_;
        std::vector<ClientPtr> clients_;
    };

    // SigInviteClientMessage 邀请其他人加入房间
    // client -> server
    struct SigInviteClientMessage : public SigBaseMessage {
        std::string client_id_;
        std::string remote_client_id_;
        std::string room_id_;
    };

    // SigOnInvitedRoomMessage 被邀请的人收到这个回调
    // server -> peer client
    struct SigOnInvitedToRoomMessage : public SigBaseMessage {
        std::string invitor_client_id_;
        std::string self_client_id_;
        std::string room_id_;
        std::vector<ClientPtr> clients_;
    };

    // SigOnRemoteInvitedRoomMessage 发起邀请的人收到这个回调
    // server -> request client
    struct SigOnRemoteInvitedToRoomMessage : public SigBaseMessage {
        std::string client_id_;
        std::string remote_client_id_;
        std::string room_id_;
        std::vector<ClientPtr> clients_;
    };

    // 请求对方信息
    struct SigReqRemoteInfoMessage : public SigBaseMessage {
        std::string client_id_;
        std::string remote_client_id_;
    };

    // 对方返回信息
    struct SigOnRemoteInfoMessage : public SigBaseMessage {
        std::string self_client_id_;
        std::string controller_client_id_;
        std::vector<std::string> local_ips_;
        std::vector<std::string> www_ips_;
        uint64_t www_ip_time_{0};
    };

    // SigHeartBeatMessage 心跳
    // client -> server
    struct SigHeartBeatMessage : public SigBaseMessage {
        std::string client_id_;
        int64_t index_;
        std::vector<std::string> local_ips_;
        std::vector<std::string> www_ips_;
    };

    // SigOnHeartBeatMessage 心跳回复
    // server -> client
    struct SigOnHeartBeatMessage : public SigBaseMessage {
        std::string client_id_;
        int64_t index_;
    };

    // SigOfferSdpMessage 客户端发过来的Sdp
    // client -> server -> remote client
    struct SigOfferSdpMessage : public SigBaseMessage {
        std::string client_id_;
        std::string room_id_;
        std::string sdp_;
        int64_t request_video_track_index = 0;
        // 用于云流化的: 来自浏览器的sessionid,用于区分是否同一个浏览器会话。
        std::string session_id;
        // 用于云流化的: 发送消息的平台标记。
        std::string platform;
    };

    // SigAnswerSdpMessage 服务端响应的Sdp
    // remote client -> server -> client
    struct SigAnswerSdpMessage : public SigBaseMessage {
        std::string client_id_;
        std::string room_id_;
        std::string sdp_;
        int peer_conn_id = 0;
        int video_track_index = 0;
    };

    // SigIceMessage 两端交互的ICE
    // client <---> remote client
    struct SigIceMessage : public SigBaseMessage {
        std::string client_id_;
        std::string room_id_;
        std::string ice_;
        std::string mid_;
        int sdp_mline_index_;
        int peer_conn_id = 0;
    };

    // 数据通道建立完成
    // 控制端先注册DataChannel，然后被控端的PeerConnection会产生回调，
    // 此时通知给控制端已经建立完成，同时发送初始化数据给控制端
    // remote client -> client
    struct SigOnDataChannelReadyMessage : public SigBaseMessage {
        std::string self_client_id_;
        std::string controller_client_id_;
        std::string room_id_;
    };

    // 请求控制对方
    struct SigReqControlMessage : public SigBaseMessage {
        std::string client_id_;
        std::string remote_client_id_;
    };

    // 对方回复已经处于控制中
    struct SigUnderControlMessage : public SigBaseMessage {
        std::string self_client_id_;
        std::string controller_client_id_;
    };

    struct SigOnRejectControlMessage : public SigBaseMessage {
        std::string client_id_;
        std::string controller_client_id_;
        std::string room_id_;
    };

    // 对方发送消息
    struct SigSendMessage : public SigBaseMessage {
        std::string client_id_;
        std::string controller_client_id_;
        std::string message_;
    };

    // 处理完成后回复对方
    struct SigOnProcessedMessage : public SigBaseMessage {
        std::string self_client_id_;
        std::string controller_client_id_;
        std::string message_;
    };

}

#endif //TEST_WEBRTC_SIG_MESSAGE_H

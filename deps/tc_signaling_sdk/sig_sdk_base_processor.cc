//
// Created by hy RGAA
//

#include "sig_sdk_base_processor.h"

#include <iostream>
#include <chrono>
#include "sig_sdk_maker.h"
#include "sig_sdk_apis.h"
#include "sig_sdk_events.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include "sig_sdk_context.h"
#include "sig_sdk_abs_router.h"
#include "sig_sdk_maker.h"

using namespace nlohmann;

namespace tc
{

    SigSdkBaseProcessor::SigSdkBaseProcessor(const std::shared_ptr<SigSdkContext>& ctx) {
        sdk_ctx_ = ctx;
    }

    void SigSdkBaseProcessor::Start(const SignalingParam& param) {
        sig_param_ = param;
        sig_router_->RegisterSigMessageCallback([=, this](const std::string &msg) {
            try {
                this->ParseSigMessage(msg);
            } catch (const std::exception &e) {
                LOGE("Parse sig message failed, error: {}, msg: {}", e.what(), msg);
            }
        });
    }

    void SigSdkBaseProcessor::Stop() {

    }

    bool SigSdkBaseProcessor::IsAlive() {
        bool alive = sig_router_ && sig_router_->IsAlive();
        return alive;
    }

    void SigSdkBaseProcessor::SendHello(const SigHelloMessage& msg) {
        if (!IsAlive()) {
            LOGE("SendHello failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeHello(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::CreateRoom(const SigCreateRoomMessage& msg) {
        if (!IsAlive()) {
            LOGE("CreateRoom failed, router not alive!");
            return;
        }
        std::string info = SigSdkMessageMaker::MakeCreateRoom(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::JoinRoom(const SigJoinRoomMessage& msg) {
        if (!IsAlive()) {
            LOGE("JoinRoom failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeJoinRoom(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::LeaveRoom(const SigLeaveRoomMessage& msg) {
        if (!IsAlive()) {
            LOGE("LeaveRoom failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeLeaveRoom(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::InviteClient(const SigInviteClientMessage& msg) {
        if (!IsAlive()) {
            LOGE("InviteClient failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeInviteRemoteClient(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::ReqRemoteInfo(const SigReqRemoteInfoMessage& msg) {
        if (!IsAlive()) {
            LOGE("ReqRemoteInfo failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeReqRemoteInfo(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::OnRemoteInfo(const SigOnRemoteInfoMessage& msg) {
        if (!IsAlive()) {
            LOGE("ReqRemoteInfo failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeOnRemoteInfo(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::SendOfferSdp(const SigOfferSdpMessage& msg) {
        if (!IsAlive()) {
            LOGE("SendOfferSdp failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeOfferSdp(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::SendAnswerSdp(const SigAnswerSdpMessage& msg) {
        if (!IsAlive()) {
            LOGE("SendAnswerSdp failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeAnswerSdp(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::SendIceCandidate(const SigIceMessage& msg) {
        if (!IsAlive()) {
            LOGE("SendIceCandidate failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeIceCandidate(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::SendDataChannelReady(const SigOnDataChannelReadyMessage& msg) {
        if (!IsAlive()) {
            LOGE("SendIceCandidate failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeDataChannelReady(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::SendReqControl(const SigReqControlMessage& msg) {
        if (!IsAlive()) {
            LOGE("SendReqControl failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeReqControl(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::SendUnderControl(const SigUnderControlMessage& msg) {
        if (!IsAlive()) {
            LOGE("SendUnderControl failed, router not alive!");
            return;
        }
        auto info = SigSdkMessageMaker::MakeUnderControl(msg);
        sig_router_->SendSigMessage(msg.sig_name_, msg.token_, info);
    }

    void SigSdkBaseProcessor::ParseSigMessage(const std::string& data) {
        json obj;
        try {
            obj = json::parse(data);
        }
        catch (const std::exception& e) {
            LOGE("Parse json failed: {}, data: {}", e.what(), data);
            return;
        }
        if (obj[kKeySigName].is_null()) {
            LOGE("Invalid message because of empty sig name");
            return;
        }
        if (obj[kKeyToken].is_null()) {
            LOGE("Invalid message because of empty token.");
            return;
        }
        std::string sig_name = obj[kKeySigName].get<std::string>();
        std::string token = obj[kKeyToken].get<std::string>();
        //RLogI("sig in: {}, msg: {}", sig_name, data);

        if (sig_name == kSigError) {
            // 收到服务异常信息
            SigErrorMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.code_ = obj[kKeySigCode].get<int>();
            msg.info_ = obj[kKeySigInfo].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);

        }
        else if (sig_name == kSigOnHello) {
            // 收到Hello回复
            SigOnHelloMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.sig_origin_msg_ = data.data();

            // 提取信令服务配置的ice_server等信息。
            auto json_ice_servers = obj[kKeyIceServers];
            if(json_ice_servers.is_array()){
                for(int i = 0; i < json_ice_servers.size(); ++i) {
                    auto json_ice_server = json_ice_servers[i];
                    SigSdkIceServer ice_server;
                    if(json_ice_server[kKeyIceUrls].is_string()) {
                        ice_server.urls = json_ice_server[kKeyIceUrls].get<std::string>();
                    }
                    if(json_ice_server[kKeyIceUsername].is_string()) {
                        ice_server.username = json_ice_server[kKeyIceUsername].get<std::string>();
                    }
                    if(json_ice_server[kKeyIceCredential].is_string()) {
                        ice_server.credential = json_ice_server[kKeyIceCredential].get<std::string>();
                    }
                    msg.ice_servers.emplace_back(ice_server);
                }
            }

            //msg_notifier_->SendAppMessage(msg);

        }
        else if (sig_name == kSigOnCreatedRoom) {
            // 创建Room回复
            SigOnCreatedRoomMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.remote_client_id_ = obj[kKeyRemoteClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);

        }
        else if (sig_name == kSigOnJoinedRoom) {
            // 加入Room回复
            SigOnJoinedRoomMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.remote_client_id_ = obj[kKeyRemoteClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            auto clients_obj = obj[kKeyClients];
            auto clients = ParseClients(clients_obj);
            msg.clients_ = clients;
            //msg_notifier_->SendAppMessage(msg);

        }
        else if (sig_name == kSigOnRemoteJoinedRoom) {
            // 其他人加入Room
            SigOnRemoteJoinedRoomMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.remote_client_id_ = obj[kKeyRemoteClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            auto clients_obj = obj[kKeyClients];
            auto clients = ParseClients(clients_obj);
            msg.clients_ = clients;
            //msg_notifier_->SendAppMessage(msg);

        }
        else if (sig_name == kSigOnLeftRoom) {
            // 自己离开Room
            SigOnLeftRoomMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            auto clients_obj = obj[kKeyClients];
            auto clients = ParseClients(clients_obj);
            msg.clients_ = clients;
            //msg_notifier_->SendAppMessage(msg);

        }
        else if (sig_name == kSigOnRemoteLeftRoom) {
            // 其他人离开Room
            SigOnRemoteLeftRoomMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.remote_client_id_ = obj[kKeyRemoteClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.clients_ = ParseClients(obj[kKeyClients]);
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);

        }
        else if (sig_name == kSigOnInvitedToRoom) {
            // 被邀请
            SigOnInvitedToRoomMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.invitor_client_id_ = obj[kKeyInvitorClientId].get<std::string>();
            msg.self_client_id_ = obj[kKeySelfClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            auto clients_obj = obj[kKeyClients];
            msg.clients_ = ParseClients(clients_obj);
            //msg_notifier_->SendAppMessage(msg);

        }
        else if (sig_name == kSigOnRemoteInvitedToRoom) {
            // 邀请其他人成功
            SigOnRemoteInvitedToRoomMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.remote_client_id_ = obj[kKeyRemoteClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            auto clients_obj = obj[kKeyClients];
            msg.clients_ = ParseClients(clients_obj);
            //msg_notifier_->SendAppMessage(msg);

        } else if (sig_name == kSigReqRemoteInfo) {
            // 请求对方的信息
            SigReqRemoteInfoMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.remote_client_id_ = obj[kKeyRemoteClientId].get<std::string>();
            //msg_notifier_->SendAppMessage(msg);

        } else if (sig_name == kSigOnRemoteInfo) {
            // 返回自己的信息给对端
            SigOnRemoteInfoMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.self_client_id_ = obj[kKeySelfClientId].get<std::string>();
            msg.controller_client_id_ = obj[kKeyControllerClientId].get<std::string>();
            msg.local_ips_ = ParseIps(obj[kKeyLocalIps]);
            msg.www_ips_ = ParseIps(obj[kKeyWwwIps]);
            //msg_notifier_->SendAppMessage(msg);

        } else if (sig_name == kSigOnHeartBeat) {
            // 心跳
            SigOnHeartBeatMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.index_ = obj[kKeyIndex].get<int64_t>();
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);

        }
        else if (sig_name == kSigOfferSdp) {
            // 收到Offer Sdp
            SigOfferSdpMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.sdp_ = obj[kKeySdp].get<std::string>();
            msg.request_video_track_index = obj[kKeyReqVideoTrackIndex].get<int64_t>();
            msg.sig_origin_msg_ = data.data();

            if(obj[kKeyPlatform].is_string())
                msg.platform = obj[kKeyPlatform].get<std::string>();
            if(obj[kKeySessionId].is_string())
                msg.session_id = obj[kKeySessionId].get<std::string>();

            //msg_notifier_->SendAppMessage(msg);

        }
        else if (sig_name == kSigAnswerSdp) {
            // 收到Answer Sdp
            SigAnswerSdpMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.sdp_ = obj[kKeySdp].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);

        }
        else if (sig_name == kSigIce) {
            // 收到Ice
            SigIceMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.ice_ = obj[kKeyIce].get<std::string>();
            msg.mid_ = obj[kKeyMid].get<std::string>();
            msg.sdp_mline_index_ = obj[kKeySdpMLineIndex].get<int>();
            msg.peer_conn_id = obj[kKeyPeerConnId].get<int>();
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);
        }
        else if (sig_name == kSigOnDataChannelReady) {
            SigOnDataChannelReadyMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.self_client_id_ = obj[kKeySelfClientId].get<std::string>();
            msg.controller_client_id_ = obj[kKeyControllerClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);
        }
        else if (sig_name == kSigReqControl) {
            SigReqControlMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.remote_client_id_ = obj[kKeyRemoteClientId].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);
        }
        else if (sig_name == kSigUnderControl) {
            SigUnderControlMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.self_client_id_ = obj[kKeySelfClientId].get<std::string>();
            msg.controller_client_id_ = obj[kKeyControllerClientId].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);
        }
        else if (sig_name == kSigOnRejectControl) {
            SigOnRejectControlMessage msg;
            msg.sig_name_ = sig_name;
            msg.token_ = token;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.controller_client_id_ = obj[kKeyControllerClientId].get<std::string>();
            msg.room_id_ = obj[kKeyRoomId].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);
        }
        else if (sig_name == kSigSendMessage) {
            SigSendMessage msg;
            msg.sig_name_ = sig_name;
            msg.client_id_ = obj[kKeyClientId].get<std::string>();
            msg.controller_client_id_ = obj[kKeyControllerClientId].get<std::string>();
            msg.message_ = obj[kKeyMessage].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);
        }
        else if (sig_name == kSigOnProcessedMessage) {
            SigOnProcessedMessage msg;
            msg.sig_name_ = sig_name;
            msg.self_client_id_ = obj[kKeySelfClientId].get<std::string>();
            msg.controller_client_id_ = obj[kKeyControllerClientId].get<std::string>();
            msg.message_ = obj[kKeyMessage].get<std::string>();
            msg.sig_origin_msg_ = data.data();
            //msg_notifier_->SendAppMessage(msg);
        }
    }

    std::vector<std::shared_ptr<Client>> SigSdkBaseProcessor::ParseClients(const nlohmann::json& obj) {
        std::vector<std::shared_ptr<Client>> clients;
        if (!obj.is_array()) {
            return clients;
        }
        for (const auto& client_obj : obj) {
            auto client = std::make_shared<Client>();
            client->id_ = client_obj[kKeyId].get<std::string>();
            client->name_ = client_obj[kKeyName].get<std::string>();
            client->role_ = client_obj[kKeyRole].get<std::string>();
            client->platform_ = client_obj[kKeyPlatform].get<std::string>();
            client->room_id_ = client_obj[kKeyRoomId].get<std::string>();
            client->update_timestamp_ = client_obj[kKeyUpdateTimestamp].get<int64_t>();
            clients.push_back(client);
        }
        return clients;
    }

    std::vector<std::string> SigSdkBaseProcessor::ParseIps(const nlohmann::json& obj) {
        std::vector<std::string> ips;
        if (!obj.is_array()) {
            return ips;
        }
        for (const auto& ip_obj : obj) {
            ips.push_back(ip_obj.get<std::string>());
        }
        return ips;
    }

    void SigSdkBaseProcessor::SendSigMessage(const std::string& sig_name, const std::string& token, const std::string& msg) {
        if (!IsAlive()) {
            return;
        }
        sig_router_->SendSigMessage(sig_name, token, msg);
    }

}
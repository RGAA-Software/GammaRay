//
// Created by hy on 2024/3/4.
//

#include "signaling_manager.h"
#include <iostream>
#include "signaling_maker.h"
#include "json/json.hpp"

using namespace nlohmann;

namespace dl
{

    std::shared_ptr<SignalingManager> SignalingManager::Make() {
        return std::make_shared<SignalingManager>();
    }

    SignalingManager::SignalingManager() {

    }

    SignalingManager::~SignalingManager() {

    }

    void SignalingManager::Start(const SignalingParam &param) {
        sig_param_ = param;
        client_ = std::make_shared<asio2::ws_client>();
        client_->set_connect_timeout(std::chrono::seconds(5));

        client_->bind_init([&]() {
            client_->ws_stream().binary(true);
        })
        .bind_connect([&]() {
            if (asio2::get_last_error()) {
                printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
                return;
            }
            else {
                printf("connect success : %s %u\n", client_->local_address().c_str(), client_->local_port());
            }

        })
        .bind_upgrade([&]() {
            if (asio2::get_last_error())
                std::cout << "upgrade failure : " << asio2::last_error_val() << " " << asio2::last_error_msg() << std::endl;
            else {
                const websocket::response_type &rep = client_->get_upgrade_response();
                auto it = rep.find(http::field::authentication_results);
                if (it != rep.end()) {
                    beast::string_view auth = it->value();
                    std::cout << auth << std::endl;
                    ASIO2_ASSERT(auth == "200 OK");
                }
            }
        })
        .bind_recv([&](std::string_view data) {
            //printf("recv : %zu %.*s\n", data.size(), (int) data.size(), data.data());
            auto sig_msg = this->ParseSignalingMessage(data);
            if (sig_msg) {
                this->DispatchSignalingMessage(std::move(sig_msg));
            }
        });

        // the /ws is the websocket upgraged target
        if (!client_->start("127.0.0.1", "8080", "/signaling")) {
            printf("connect websocket server failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }
    }

    void SignalingManager::Join(const std::string& roomId, const std::string& sessionId) {
        if (!client_ || !client_->is_started()) {
            return;
        }
        auto msg = SignalingMaker::MakeJoinMessage(roomId, sessionId);
        client_->async_send(msg);
    }

    void SignalingManager::Leave(const std::string& roomId, const std::string& sessionId) {
        if (!client_ || !client_->is_started()) {
            return;
        }
        auto msg = SignalingMaker::MakeLeaveMessage(roomId, sessionId);
        client_->async_send(msg);
    }

    void SignalingManager::SendSdp(const std::string& roomId, const std::string& sessionId, const std::string& sdp) {
        if (!client_ || !client_->is_started()) {
            return;
        }
        auto msg = SignalingMaker::MakeSdpMessage(roomId, sessionId, sdp);
        client_->async_send(msg);
    }

    void SignalingManager::SendIceCandidate(const std::string& roomId, const std::string& sessionId, const std::string& ice,
                                            const std::string& sdp_mid, int sdp_mline_index) {
        if (!client_ || !client_->is_started()) {
            return;
        }
        auto msg = SignalingMaker::MakeIceCandidate(roomId, sessionId, ice, sdp_mid, sdp_mline_index);
        client_->async_send(msg);
    }

    std::shared_ptr<SignalingMessage> SignalingManager::ParseSignalingMessage(std::string_view data) {
        auto msg = std::make_shared<SignalingMessage>();
        json obj;
        try {
            obj = json::parse(data);
        } catch(const std::exception& e) {
            std::cerr << "parse json error : " << e.what() << std::endl;
            return nullptr;
        }
        msg->room_id_ = obj["room_id"].get<std::string>();
        msg->session_id_ = obj["session_id"].get<std::string>();
        msg->signal_ = obj["signal"].get<std::string>();
        msg->message_ = obj["message"].get<std::string>();
        if (!obj["sdp_mid"].is_null()) {
            msg->sdp_mid_ = obj["sdp_mid"].get<std::string>();
        }
        if (!obj["sdp_mline_index"].is_null()) {
            msg->sdp_mline_index_ = obj["sdp_mline_index"].get<int>();
        }
        return std::move(msg);
    }

    void SignalingManager::DispatchSignalingMessage(std::shared_ptr<SignalingMessage>&& msg) {
        std::cout << "will dispatch signal: " << msg->signal_ << std::endl;
        if (msg->signal_ == "joined") {
            on_joined_cbk_(std::move(msg));
        }
        else if (msg->signal_ == "other-joined") {
            on_other_joined_cbk_(std::move(msg));
        }
        else if (msg->signal_ == "leave") {
            on_leave_cbk_(std::move(msg));
        }
        else if (msg->signal_ == "other-leaved") {
            on_other_leave_cbk_(std::move(msg));
        }
        else if (msg->signal_ == "sdp") {
            on_sdp_cbk_(std::move(msg));
        }
        else if (msg->signal_ == "ice") {
            on_ice_candidate_cbk_(std::move(msg));
        }
    }
}

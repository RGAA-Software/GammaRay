//
// Created by RGAA on 2024/5/29.
//

#ifndef TC_WEBRTC_RTC_DEFINATIONS_H
#define TC_WEBRTC_RTC_DEFINATIONS_H

#include <string>

namespace tc {

struct RtcIceServer {
	std::string urls;
	std::string username;
	std::string credential;
};

class RtcContextParam {
public:
    // syxmsg.xyz
    std::string sig_host_{};
    // 9999
    int sig_port_{0};
    // sig path: [/rd/ipc] or [/signaling]
    std::string sig_path_{};
    // app key
    std::string appkey_{};
    // syxmsg.xyz:3478
    std::string stun_address_{};
    // syxmsg.xyz:3478
    std::string turn_address_{};
    // syxmsg.xyz:5349
    std::string turn_tls_address_{};
    // username
    std::string turn_username_{};
    // password
    std::string turn_password_{};
    // min port
    int min_port_{0};
    // max port
    int max_port_{0};
    // exclusive_encoder
    bool exclusive_encoder_{false};
    // client id
    std::string client_id_{};
    // remote client id
    std::string remote_client_id_{};
    // group id
    std::string group_id_{};
    // user id
    std::string user_id_{};
    // request client id
    bool request_client_id_{ false };
};

}

#endif //TC_WEBRTC_RTC_DEFINATIONS_H

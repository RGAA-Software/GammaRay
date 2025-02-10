//
// Created by RGAA on 2024/5/28.
//

#ifndef TC_WEBRTC_RTC_CONTEXT_H
#define TC_WEBRTC_RTC_CONTEXT_H

#include <memory>
#include <functional>

#include "tc_common_new/message_notifier.h"

namespace tc
{

    class Thread;
    class KvStorage;
    class Client;

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

    class RtcContext {
    public:

        static std::shared_ptr<RtcContext> Make();

        RtcContext();
        ~RtcContext();

        void Init(const RtcContextParam& param);
        void PostNetworkTask(std::function<void()>&& task);
        void PostBgTask(std::function<void()>&& task);

        void SetRandomPwd(const std::string& pwd);
        std::string GetRandomPwd();
        std::shared_ptr<MessageNotifier> GetMessageNotifier();
        RtcContextParam GetRtcContextParam();
        void SetGroupId(const std::string& id);
        std::string GetGroupId();
        void SetUserId(const std::string& id);
        std::string GetUserId();
        std::vector<std::string> GetLocalIps();
        void SetWwwIps(const std::vector<std::string>& ips);
        std::vector<std::string> GetWwwIps();

        template<typename T>
        void SendAppMessage(const T& m) {
            if (msg_notifier_) {
                msg_notifier_->SendAppMessage(m);
            }
        }

        void OnTimer1SCallback();
        void OnTimer5SCallback();

        void set_ice_servers(const std::vector<RtcIceServer>& ice_servers) { rtc_ice_servers_ = ice_servers; }
        std::vector<RtcIceServer> ice_servers() { return rtc_ice_servers_; }
    private:
        std::string RequestClientId() const;
        void QueryLocalIps();

    private:
        RtcContextParam rtc_ctx_param_;
        KvStorage* kv_storage_ = nullptr;
        std::string client_id_;
        std::string random_pwd_;
        std::string appkey_ = "test_app_key";
        std::string group_id_;
        std::string user_id_;
        std::shared_ptr<Thread> net_thread_ = nullptr;
        std::shared_ptr<Thread> bg_thread_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::vector<std::string> local_ips_;
        std::vector<std::string> www_ips_;
        // 从信令服务返回的Ice配置信息, 0端口场景中使用信令服务返回的配置。
        std::vector<RtcIceServer> rtc_ice_servers_;
    };

}

#endif //TC_WEBRTC_RTC_CONTEXT_H

//
// Created by RGAA
//

#ifndef TC_WEBRTC_RTC_CONTEXT_H
#define TC_WEBRTC_RTC_CONTEXT_H

#include <memory>
#include <functional>

#include "tc_common_new/message_notifier.h"

namespace tc
{

    class Thread;

    struct SigSdkIceServer {
        std::string urls;
        std::string username;
        std::string credential;
    };

    class SigSdkContextParam {
    public:
        std::string sig_host_{};
        int sig_port_{0};
        std::string sig_path_{};
        std::string stun_address_{};
        std::string turn_address_{};
        std::string turn_tls_address_{};
        std::string turn_username_{};
        std::string turn_password_{};
        int min_port_{0};
        int max_port_{0};
        std::string client_id_{};
        std::string remote_client_id_{};
    };

    class SigSdkClientId {
    public:
        std::string id_;
        std::string random_pwd_;

    public:
        [[nodiscard]] bool IsValid() const {
            return !id_.empty() && !random_pwd_.empty();
        }
    };

    class SigSdkContext {
    public:

        static std::shared_ptr<SigSdkContext> Make();

        SigSdkContext();
        ~SigSdkContext();

        void Init(const SigSdkContextParam& param);
        void PostNetworkTask(std::function<void()>&& task);
        void PostBgTask(std::function<void()>&& task);
        SigSdkClientId RequestNewClientId();
        void SetRandomPwd(const std::string& pwd);
        std::string GetClientId();
        std::string GetRandomPwd();
        std::shared_ptr<MessageNotifier> GetMessageNotifier();
        SigSdkContextParam GetRtcContextParam();
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

    private:
        void QueryLocalIps();

    private:
        SigSdkContextParam rtc_ctx_param_;
        std::string client_id_;
        std::string random_pwd_;
        std::string group_id_;
        std::string user_id_;
        std::shared_ptr<Thread> net_thread_ = nullptr;
        std::shared_ptr<Thread> bg_thread_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::vector<std::string> local_ips_;
        std::vector<std::string> www_ips_;
    };

}

#endif //TC_WEBRTC_RTC_CONTEXT_H

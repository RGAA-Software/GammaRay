//
// Created by RGAA on 2024/5/28.
//

#ifndef TC_WEBRTC_RTC_CONTEXT_H
#define TC_WEBRTC_RTC_CONTEXT_H

#include <memory>
#include <functional>

#include "tc_common_new/message_notifier.h"
#include "rtc_definations.h"

namespace tc {

class Thread;
class KvStorage;
class Client;

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
    void SendClientIdEvent();
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

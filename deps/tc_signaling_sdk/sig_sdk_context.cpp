//
// Created by RGAA on 2024/5/28.
//

#include "sig_sdk_context.h"
#include "sig_sdk_apis.h"
#include "json/json.hpp"
#include "tc_common_new/log.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/message_notifier.h"

using namespace nlohmann;

namespace tc
{

    std::shared_ptr<RtcContext> RtcContext::Make() {
        return std::make_shared<RtcContext>();
    }

    RtcContext::RtcContext() {
        msg_notifier_ = std::make_shared<MessageNotifier>();

        net_thread_ = std::make_shared<Thread>("network", 128);
        net_thread_->Poll();

        bg_thread_ = std::make_shared<Thread>("network", 128);
        bg_thread_->Poll();
    }

    RtcContext::~RtcContext() {

    }

    void RtcContext::Init(const RtcContextParam& param) {
        rtc_ctx_param_ = param;
        LOGI("rtc context params:");
        LOGI("sig host: {}", rtc_ctx_param_.sig_host_);
        LOGI("sig port: {}", rtc_ctx_param_.sig_port_);
        LOGI("app key: {}", rtc_ctx_param_.appkey_);
        LOGI("min port: {}", rtc_ctx_param_.min_port_);
        LOGI("max port: {}", rtc_ctx_param_.max_port_);
        QueryLocalIps();

    }

    void RtcContext::SetRandomPwd(const std::string& pwd) {
        if (!pwd.empty()) {
            random_pwd_ = pwd;
        }
    }

    std::string RtcContext::GetRandomPwd() {
        return random_pwd_;
    }

    void RtcContext::PostNetworkTask(std::function<void()>&& task) {
        net_thread_->Post(SimpleThreadTask::Make(std::move(task)));
    }

    void RtcContext::PostBgTask(std::function<void()>&& task) {
        bg_thread_->Post(SimpleThreadTask::Make(std::move(task)));
    }

    std::shared_ptr<MessageNotifier> RtcContext::GetMessageNotifier() {
        return msg_notifier_;
    }

    RtcContextParam RtcContext::GetRtcContextParam() {
        return rtc_ctx_param_;
    }

    std::string RtcContext::RequestClientId() const {
    //	SigRespStr<bool> result =
    //		SigRequest::Make(rtc_ctx_param_.sig_host_, rtc_ctx_param_.sig_port_, rtc_ctx_param_.appkey_)
    //		->RequestClientId();
    //	if (!result.result_) {
    //		RLogE("RequestClientId failed!");
    //	}
    //	return result.value_;
        return "";
    }

    void RtcContext::OnTimer1SCallback() {

    }

    void RtcContext::OnTimer5SCallback() {
        PostBgTask([=]() {
            QueryLocalIps();
        });
    }

    void RtcContext::SetGroupId(const std::string& id) {
        group_id_ = id;
    }

    std::string RtcContext::GetGroupId() {
        return group_id_;
    }

    void RtcContext::SetUserId(const std::string& id) {
        user_id_ = id;
    }

    std::string RtcContext::GetUserId() {
        return user_id_;
    }

    std::vector<std::string> RtcContext::GetLocalIps() {
        return local_ips_;
    }

    void RtcContext::SetWwwIps(const std::vector<std::string>& ips) {
        www_ips_ = ips;
    }

    std::vector<std::string> RtcContext::GetWwwIps() {
        return www_ips_;
    }

    void RtcContext::QueryLocalIps() {
        local_ips_.clear();
    //    auto ips = DLIpUtils::ScanIps(true);
    //    for (auto& ip : ips) {
    //        local_ips_.push_back(ip->ip_);
    //    }
    }

}

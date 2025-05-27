//
// Created by RGAA
//

#include "sig_sdk_context.h"
#include "sig_sdk_apis.h"
#include "json/json.hpp"
#include "tc_common_new/log.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/http_client.h"
#include "tc_common_new/hardware.h"
#include "tc_common_new/ip_util.h"
#include "tc_common_new/base64.h"

using namespace nlohmann;

namespace tc
{

    std::shared_ptr<SigSdkContext> SigSdkContext::Make() {
        return std::make_shared<SigSdkContext>();
    }

    SigSdkContext::SigSdkContext() {
        msg_notifier_ = std::make_shared<MessageNotifier>();

        net_thread_ = std::make_shared<Thread>("network", 32);
        net_thread_->Poll();

        bg_thread_ = std::make_shared<Thread>("bg_thread", 32);
        bg_thread_->Poll();
    }

    SigSdkContext::~SigSdkContext() {

    }

    void SigSdkContext::Init(const SigSdkContextParam& param) {
        rtc_ctx_param_ = param;
        LOGI("rtc context params:");
        LOGI("sig host: {}", rtc_ctx_param_.sig_host_);
        LOGI("sig port: {}", rtc_ctx_param_.sig_port_);
        LOGI("min port: {}", rtc_ctx_param_.min_port_);
        LOGI("max port: {}", rtc_ctx_param_.max_port_);
        QueryLocalIps();
    }

    void SigSdkContext::SetClientId(const std::string& id) {
        if (!id.empty()) {
            client_id_ = id;
        }
    }

    void SigSdkContext::SetRandomPwd(const std::string& pwd) {
        if (!pwd.empty()) {
            random_pwd_ = pwd;
        }
    }

    std::string SigSdkContext::GetClientId() {
        return client_id_;
    }

    std::string SigSdkContext::GetRandomPwd() {
        return random_pwd_;
    }

    void SigSdkContext::PostNetworkTask(std::function<void()>&& task) {
        net_thread_->Post(SimpleThreadTask::Make(std::move(task)));
    }

    void SigSdkContext::PostBgTask(std::function<void()>&& task) {
        bg_thread_->Post(SimpleThreadTask::Make(std::move(task)));
    }

    std::shared_ptr<MessageNotifier> SigSdkContext::GetMessageNotifier() {
        return msg_notifier_;
    }

    SigSdkContextParam SigSdkContext::GetRtcContextParam() {
        return rtc_ctx_param_;
    }

    SigSdkClientId SigSdkContext::RequestNewClientId() {
        auto hardware_desc = Hardware::Instance()->GetHardwareDescription();
        auto et_info = IPUtil::ScanIPs();
        std::string mac_address;
        for (auto& item : et_info) {
            if (!item.mac_address_.empty() && mac_address.find(item.mac_address_) != std::string::npos) {
                continue;
            }
            mac_address = mac_address.append(item.mac_address_);
        }
        if (hardware_desc.empty()) {
            return {};
        }
        auto client = HttpClient::Make(rtc_ctx_param_.sig_host_, rtc_ctx_param_.sig_port_, kSigApiReqClientId);
        auto resp = client->Request({
#ifdef WIN32
            {"platform", "windows"},
#endif
            {"hardware", hardware_desc + Base64::Base64Encode(mac_address)},
        });
        if (resp.status != 200 || resp.body.empty()) {
            LOGE("Request for steam games failed.");
            return {};
        }
        try {
            auto obj = json::parse(resp.body);
            auto client_id = obj["value"]["id"].get<std::string>();
            auto random_pwd = obj["value"]["random_pwd"].get<std::string>();
            LOGI("RequestNewClientId: {} => {}", client_id, random_pwd);
            return SigSdkClientId {
                .id_ = client_id,
                .random_pwd_ = random_pwd,
            };
        } catch(std::exception& e) {
            LOGE("RequestNewClientId failed: {}, message: {}", e.what(), resp.body);
            return {};
        }
    }

    void SigSdkContext::OnTimer1SCallback() {

    }

    void SigSdkContext::OnTimer5SCallback() {
        PostBgTask([=]() {
            QueryLocalIps();
        });
    }

    std::vector<std::string> SigSdkContext::GetLocalIps() {
        return local_ips_;
    }

    void SigSdkContext::SetWwwIps(const std::vector<std::string>& ips) {
        www_ips_ = ips;
    }

    std::vector<std::string> SigSdkContext::GetWwwIps() {
        return www_ips_;
    }

    void SigSdkContext::QueryLocalIps() {
        local_ips_.clear();
    //    auto ips = DLIpUtils::ScanIps(true);
    //    for (auto& ip : ips) {
    //        local_ips_.push_back(ip->ip_);
    //    }
    }

}

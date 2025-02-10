//
// Created by RGAA on 2024/3/4.
//

#include "sig_wss_manager.h"
#include <iostream>
#include <chrono>
#include "sig_maker.h"
#include "sig_apis.h"
#include "sig_events.h"
#include "tc_common_new/log.h"
#include "rtc_context.h"
#include "sig_router_wss_impl.h"

using namespace nlohmann;

namespace tc {

std::shared_ptr<SigWssManager> SigWssManager::Make(const std::shared_ptr<RtcContext>& ctx) {
    return std::make_shared<SigWssManager>(ctx);
}

SigWssManager::SigWssManager(const std::shared_ptr<RtcContext>& ctx) : SigMgrInterface(ctx) {
    
}

SigWssManager::~SigWssManager() {

}

void SigWssManager::Start(const SignalingParam& param) {
    sig_router_ = std::make_shared<SigRouterWssImpl>(rtc_ctx_);
    SigMgrInterface::Start(param);
    sig_router_->Init(param);
    sig_router_->Start();
    LOGI("SigWssManager start...");
}

void SigWssManager::Stop() {
    if (sig_router_) {
        sig_router_->Exit();
    }
}

}

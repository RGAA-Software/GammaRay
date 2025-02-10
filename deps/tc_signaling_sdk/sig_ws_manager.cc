#include "sig_ws_manager.h"

#include <iostream>
#include <chrono>

#include "sig_maker.h"
#include "sig_apis.h"
#include "sig_events.h"
#include "tc_common_new/log.h"
#include "rtc_context.h"
#include "sig_router_ws_impl.h"

using namespace nlohmann;

namespace tc {

std::shared_ptr<SigWsManager> SigWsManager::Make(const std::shared_ptr<RtcContext>& ctx) {
    return std::make_shared<SigWsManager>(ctx);
}

SigWsManager::SigWsManager(const std::shared_ptr<RtcContext>& ctx) : SigMgrInterface(ctx) {
    
}

SigWsManager::~SigWsManager() {

}

void SigWsManager::Start(const SignalingParam& param) {
    sig_router_ = std::make_shared<SigRouterWsImpl>(rtc_ctx_);
    SigMgrInterface::Start(param);
    sig_router_->Init(param);
    sig_router_->Start();
    LOGI("SigWsManager start...");
}

void SigWsManager::Stop() {
    if (sig_router_) {
        sig_router_->Exit();
    }
}

}

//
// Created by hy RGAA
//
#include "sig_sdk_ws_processor.h"

#include <iostream>
#include <chrono>

#include "sig_sdk_maker.h"
#include "sig_sdk_apis.h"
#include "sig_sdk_events.h"
#include "tc_common_new/log.h"
#include "sig_sdk_context.h"
#include "sig_sdk_ws_router.h"

using namespace nlohmann;

namespace tc
{

    std::shared_ptr<SigSdkWsProcessor> SigSdkWsProcessor::Make(const std::shared_ptr<SigSdkContext>& ctx) {
        return std::make_shared<SigSdkWsProcessor>(ctx);
    }

    SigSdkWsProcessor::SigSdkWsProcessor(const std::shared_ptr<SigSdkContext>& ctx) : SigSdkBaseProcessor(ctx) {

    }

    SigSdkWsProcessor::~SigSdkWsProcessor() {

    }

    void SigSdkWsProcessor::Start(const SignalingParam& param) {
        sig_router_ = std::make_shared<SigSdkWsRouter>(rtc_ctx_);
        SigSdkBaseProcessor::Start(param);
        sig_router_->Init(param);
        sig_router_->Start();
        LOGI("SigWsManager start...");
    }

    void SigSdkWsProcessor::Stop() {
        if (sig_router_) {
            sig_router_->Exit();
        }
    }

}

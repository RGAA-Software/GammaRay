//
// Created by RGAA
//
#pragma once

#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <asio2/asio2.hpp>
#include "sig_sdk_base_processor.h"

namespace tc
{

    class SigSdkContext;
    class MessageNotifier;

    class SigSdkWsProcessor : public SigSdkBaseProcessor {
    public:

        static std::shared_ptr<SigSdkWsProcessor> Make(const std::shared_ptr<SigSdkContext>& ctx);

        explicit SigSdkWsProcessor(const std::shared_ptr<SigSdkContext>& ctx);
        ~SigSdkWsProcessor();
        void Start(const SignalingParam& param) override;
        void Stop() override;

    private:

    };

}


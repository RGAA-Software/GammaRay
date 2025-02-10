#pragma once

#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <asio2/asio2.hpp>
#include "sig_sdk_mgr_interface.h"

namespace tc
{

    class RtcContext;
    class MessageNotifier;

    class SigWsManager : public SigMgrInterface {
    public:

        static std::shared_ptr<SigWsManager> Make(const std::shared_ptr<RtcContext>& ctx);

        explicit SigWsManager(const std::shared_ptr<RtcContext>& ctx);
        ~SigWsManager();
        void Start(const SignalingParam& param) override;
        void Stop() override;

    private:

    };

}


//
// Created by RGAA on 2024/3/4.
//

#ifndef TC_WEBRTC_SIG_MANAGER_H
#define TC_WEBRTC_SIG_MANAGER_H

#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <asio2/asio2.hpp>
#include "sig_mgr_interface.h"

namespace tc {

class RtcContext;
class MessageNotifier;

class SigWssManager : public SigMgrInterface {
public:

    static std::shared_ptr<SigWssManager> Make(const std::shared_ptr<RtcContext>& ctx);

    explicit SigWssManager(const std::shared_ptr<RtcContext>& ctx);
    ~SigWssManager();
    void Start(const SignalingParam& param) override;
    void Stop() override;

private:
    
};

}

#endif //TC_WEBRTC_SIG_MANAGER_H

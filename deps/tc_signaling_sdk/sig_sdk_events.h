//
// Created by RGAA
//

#ifndef TC_WEBRTC_SIG_EVENTS_H
#define TC_WEBRTC_SIG_EVENTS_H

#include <string>
#include <cstdint>

#include "sig_sdk_message.h"

namespace tc
{

    class SigEvtOnConnect {
    public:
        bool connected_;
    };

    class SigEvtOnDisconnect {
    public:

    };

    class SigEvtTimer1S {
    public:

    };

    class SigEvtTimer2S {
    public:

    };

    class SigEvtTimer5S {

    };

    // data channel 发送的消息
    class SigEvtControl {
    public:
        int peer_id_;
        std::string event_;
        std::string token_;
    };

}
#endif //TC_WEBRTC_SIG_EVENTS_H

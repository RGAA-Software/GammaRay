//
// Created by RGAA on 2024/5/30.
//

#ifndef TC_WEBRTC_CLIENT_RTC_BASE_EVENTS_H
#define TC_WEBRTC_CLIENT_RTC_BASE_EVENTS_H

#include <string>

namespace tc
{

    class BaseEvtRequestedId {
    public:
        std::string client_id_;
        std::string random_pwd_;
    };


}

#endif //TC_WEBRTC_CLIENT_RTC_BASE_EVENTS_H

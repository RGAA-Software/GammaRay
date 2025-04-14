//
// Created by RGAA on 14/04/2025.
//

#ifndef GAMMARAY_RTC_MESSAGES_H
#define GAMMARAY_RTC_MESSAGES_H

#include <string>

namespace tc
{

    class MsgRtcRemoteSdp {
    public:
        std::string stream_id_;
        std::string device_id_;
        std::string sdp_;
    };

    class MsgRtcRemoteIce {
    public:
        std::string stream_id_;
        std::string device_id_;
        std::string ice_;
        std::string mid_;
        int sdp_mline_index_ = 0;
    };

}

#endif //GAMMARAY_RTC_MESSAGES_H

//
// Created by RGAA on 2024/3/5.
//

#ifndef TEST_WEBRTC_SIGNALING_MAKER_H
#define TEST_WEBRTC_SIGNALING_MAKER_H

#include "json/json.hpp"

namespace dl
{

    class SignalingMaker {
    public:

        static std::string MakeJoinMessage(const std::string& roomId, const std::string& sessionId);
        static std::string MakeLeaveMessage(const std::string& roomId, const std::string& sessionId);
        static std::string MakeSdpMessage(const std::string& roomId, const std::string& sessionId, const std::string& sdp);
        static std::string MakeIceCandidate(const std::string& roomId, const std::string& sessionId,
                                            const std::string& ice, const std::string& sdp_mid, int sdp_mline_index);

    };

}

#endif //TEST_WEBRTC_SIGNALING_MAKER_H

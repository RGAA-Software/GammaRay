//
// Created by RGAA on 2024/3/5.
//

#include "signaling_maker.h"

using namespace nlohmann;

namespace dl
{

    std::string SignalingMaker::MakeJoinMessage(const std::string& roomId, const std::string& sessionId) {
        json obj;
        obj["room_id"] = roomId;
        obj["session_id"] = sessionId;
        obj["signal"] = "join";
        obj["message"] = "";
        return obj.dump(2);
    }

    std::string SignalingMaker::MakeLeaveMessage(const std::string& roomId, const std::string& sessionId) {
        json obj;
        obj["room_id"] = roomId;
        obj["session_id"] = sessionId;
        obj["signal"] = "leave";
        obj["message"] = "";
        return obj.dump(2);
    }

    std::string SignalingMaker::MakeSdpMessage(const std::string& roomId, const std::string& sessionId, const std::string& sdp) {
        json obj;
        obj["room_id"] = roomId;
        obj["session_id"] = sessionId;
        obj["signal"] = "sdp";
        obj["message"] = sdp;
        return obj.dump(2);
    }

    std::string SignalingMaker::MakeIceCandidate(const std::string& roomId, const std::string& sessionId, const std::string& ice,
                                                 const std::string& sdp_mid, int sdp_mline_index) {
        json obj;
        obj["room_id"] = roomId;
        obj["session_id"] = sessionId;
        obj["signal"] = "ice";
        obj["message"] = ice;
        obj["sdp_mid"] = sdp_mid;
        obj["sdp_mline_index"] = sdp_mline_index;
        return obj.dump(2);
    }


}
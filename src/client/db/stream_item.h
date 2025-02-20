//
// Created by RGAA on 2023/8/14.
//

#ifndef SAILFISH_CLIENT_PC_STREAMITEM_H
#define SAILFISH_CLIENT_PC_STREAMITEM_H

#include <string>

namespace tc
{

    static std::string kStreamItemNtTypeWebSocket = "websocket";
    static std::string kStreamItemNtTypeUdpKcp = "udp_kcp";
    static std::string kStreamItemNtTypeWebRTCDirect = "webrtc_direct";
    static std::string kStreamItemNtTypeWebRTC = "webrtc";

    // WARN: Ported from another project, some fields are deprecated.
    class StreamItem {
    public:
        [[nodiscard]] bool IsValid() const;

    public:

        int _id = 0;

        // stream id
        std::string stream_id;

        // stream name
        std::string stream_name;

        // encode bitrate, for example : 5, that means 5Mbps
        int encode_bps = 5;

        // audio capture status
        int audio_enabled = true;

        // audio source, global / app_only
        std::string audio_capture_mode;;

        std::string stream_host;
        int stream_port = 9002;
        int bg_color = 0;

        int encode_fps;

        // network type
        // websocket / udp_kcp / webrtc_direct / webrtc
        std::string network_type_;

        // 9 numbers
        std::string client_id_;

        // random password
        std::string client_random_pwd_;

        // safety password
        std::string client_safety_pwd_;

    };

}

#endif //SAILFISH_CLIENT_PC_STREAMITEM_H

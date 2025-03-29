//
// Created by RGAA on 2023/8/14.
//

#ifndef SAILFISH_CLIENT_PC_STREAMITEM_H
#define SAILFISH_CLIENT_PC_STREAMITEM_H

#include <string>
#include "client/ct_stream_item_net_type.h"

namespace tc
{

    // WARN: Ported from another project, some fields are deprecated.
    class StreamItem {
    public:
        [[nodiscard]] bool IsValid() const;
        [[nodiscard]] bool IsRelay() const;

    public:

        int _id = 0;

        // stream id
        std::string stream_id_;

        // stream name
        std::string stream_name_;

        // encode bitrate, for example : 5, that means 5Mbps
        int encode_bps_ = 5;

        // audio capture status
        int audio_enabled_ = true;

        // audio source, global / app_only
        std::string audio_capture_mode_;;

        std::string stream_host_;
        int stream_port_ = 9002;
        int bg_color_ = 0;

        int encode_fps_;

        // network type
        // websocket / udp_kcp / webrtc_direct / webrtc
        std::string network_type_;

        // direct / signaling
        std::string connect_type_;

        // 9 numbers
        std::string device_id_;

        // random password
        std::string device_random_pwd_;

        // safety password
        std::string device_safety_pwd_;

        // remote device id
        std::string remote_device_id_;

        // remote device random pwd
        std::string remote_device_random_pwd_;

        // remote device safety pwd
        std::string remote_device_safety_pwd_;

        // created timestamp
        int64_t created_timestamp_ {0};

        // update timestamp
        int64_t updated_timestamp_ {0};
    };

}

#endif //SAILFISH_CLIENT_PC_STREAMITEM_H

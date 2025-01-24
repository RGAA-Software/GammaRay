//
// Created by RGAA on 2023/8/14.
//

#ifndef SAILFISH_CLIENT_PC_STREAMITEM_H
#define SAILFISH_CLIENT_PC_STREAMITEM_H

#include <string>

namespace tc
{

    // ATTENTION : This class is ported from my another project, using only some fields.

    class StreamItem {
    public:


        bool IsValid() const;

    public:

        int _id = 0;

        // stream id
        std::string stream_id;

        // stream name
        std::string stream_name;

        // target: game / monitor
        std::string stream_target;

        // if target = monitor, gdi / dda / graphics
        std::string monitor_capture_method;

        // if target = game, the game's exe path
        std::string exe_path;

        // image capture mode, hook / window-capture
        std::string capture_mode;

        // image encode type, h264 / h265 / vp9
        std::string encoder_type;

        // encoder hardware, gpu / cpu, if no gpu exist int the machine, use CPU
        std::string encoder_hw;

        // encode bitrate, for example : 5, that means 5Mbps
        int encode_bps = 5;

        // audio capture status
        int audio_enabled = true;

        // audio source, global / app_only
        std::string audio_capture_mode;

        // gpu router status
        int gpu_router_enabled = false;

        // router policy, circulation / hardware_usage
        std::string gpu_router_policy;

        // frame resize in the server, before transfer to client
        int frame_resize_enabled = false;

        int frame_resize_width = 0;
        int frame_resize_height = 0;

        // way to replay mouse/keyboard events, global / app_only
        std::string replay_mode;

        std::string stream_host;

        // websocket listening port
        int stream_port = 9002;

        // pass params to the running target, for examples: -D3D12 to force d3d version...
        std::string app_args;

        // if there is no connections, auto exit after a period time
        int auto_exit = true;

        // see auto_exit
        int auto_exit_period = 10;

        // all players can operate the same game/app/monitor
        int enable_multi_players = true;

        int bg_color = 0;

        int encode_fps;

    };

}

#endif //SAILFISH_CLIENT_PC_STREAMITEM_H

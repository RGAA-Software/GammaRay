//
// Created by RGAA on 2024/4/10.
//

#ifndef TC_SERVER_STEAM_GR_SETTINGS_H
#define TC_SERVER_STEAM_GR_SETTINGS_H

#include <vector>
#include <string>

namespace tc
{

    static const std::string kStLogFile = "logfile";
    static const std::string kStEncoderSelectType = "encoder_select_type";
    static const std::string kStEncoderName = "encoder_name";
    static const std::string kStEncoderFormat = "encoder_format";
    static const std::string kStEncoderBitrate = "encoder_bitrate";
    static const std::string kStEncoderResolutionType = "encoder_resolution_type";
    static const std::string kStEncoderWidth = "encoder_width";
    static const std::string kStEncoderHeight = "encoder_height";

    static const std::string kStCaptureAudio = "capture_audio";
    static const std::string kStCaptureAudioType = "capture_audio_type";
    static const std::string kStCaptureVideo = "capture_video";
    static const std::string kStCaptureVideoType = "capture_video_type";

    static const std::string kStNetworkType = "network_type";
    static const std::string kStNetworkListenPort = "network_listen_port";

    static const std::string kStAppGamePath = "app_game_path";
    static const std::string kStAppGameArgs = "app_game_args";

    static const std::string kStHttpPort = "k_http_port";
    static const std::string kStWsPort = "k_ws_port";

    class SharedPreference;

    class GrSettings {
    public:

        static GrSettings* Instance() {
            static GrSettings st;
            return &st;
        }

        void Load();
        void Dump();
        [[nodiscard]] std::vector<std::string> GetArgs() const;

    public:

        SharedPreference* sp_ = nullptr;

        int http_server_port_{0};
        int ws_server_port_{0};
        int udp_server_port_{0};

        std::string log_file_;
        std::string encoder_select_type_;
        std::string encoder_name_;
        std::string encoder_format_;
        std::string encoder_bitrate_;
        std::string encoder_resolution_type_;
        std::string encoder_width_;
        std::string encoder_height_;

        std::string capture_audio_;
        std::string capture_audio_type_;
        std::string capture_video_;
        std::string capture_video_type_;

        std::string network_type_;
        int network_listen_port_;

    };

}

#endif //TC_SERVER_STEAM_GR_SETTINGS_H

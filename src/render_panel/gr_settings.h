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
    static const std::string kStWebSocketEnabled = "websocket_enabled";
    static const std::string kStWebRTCEnabled = "webrtc_enabled";
    static const std::string kStUdpKcpEnabled = "udp_kcp_enabled";
    static const std::string kStNetworkListenPort = "network_listen_port";
    static const std::string kStUdpListenPort = "udp_listen_port";
    static const std::string kStAppGamePath = "app_game_path";
    static const std::string kStAppGameArgs = "app_game_args";
    static const std::string kStDebugBlock = "debug_block";
    static const std::string kStMockVideo = "mock_video";
    static const std::string kStPanelListeningPort = "panel_listen_port";
    static const std::string kStCaptureMonitor = "capture_monitor";
    static const std::string kStCaptureAudioDevice = "capture_audio_device";
    static const std::string kStFileTransferFolder = "file_transfer_folder";
    static const std::string kStListeningIp = "listening_ip";
    static const std::string kStSigServerAddress = "sig_server_address";
    static const std::string kStSigServerPort = "sig_server_port";
    static const std::string kStCoturnAddress = "coturn_server_address";
    static const std::string kStCoturnPort = "coturn_server_port";
    static const std::string kStDeviceId = "device_id";
    static const std::string kStDeviceRandomPwd = "device_random_pwd";
    static const std::string kStIDServerHost = "id_server_host";
    static const std::string kStIDServerPort = "id_server_port";
    static const std::string kStRelayServerHost = "relay_server_host";
    static const std::string kStRelayServerPort = "relay_server_port";

    static const std::string kStTrue = "true";
    static const std::string kStFalse = "false";
    static const std::string kEncFormatH264 = "h264";
    static const std::string kEncFormatH265 = "h265";
    static const std::string kResTypeOrigin = "origin";
    static const std::string kResTypeResize = "resize";

    static const std::string kGammaRayName = "GammaRay.exe";
    static const std::string kGammaRayRenderName = "GammaRayRender.exe";
    static const std::string kGammaRayClient = "GammaRayClient.exe";
    static const std::string kGammaRayClientInner = "GammaRayClientInner.exe";

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

        void SetBitrate(int br);
        void SetEnableResResize(bool enabled);
        void SetResWidth(int width);
        void SetResHeight(int height);
        void SetEncoderFormat(int idx);
        void SetCaptureVideo(bool enabled);
        void SetCaptureAudio(bool enabled);
        void SetCaptureAudioDeviceId(const std::string& name);
        [[nodiscard]] bool IsEncoderResTypeOrigin() const;
        void SetFileTransferFolder(const std::string& path);
        void SetListeningIp(const std::string& ip);
        void SetWebSocketEnabled(bool enabled);
        void SetWebRTCEnabled(bool enabled);
        void SetUdpKcpEnabled(bool enabled);
        void SetSigServerAddress(const std::string& address);
        void SetSigServerPort(const std::string& port);
        void SetCoturnServerAddress(const std::string& address);
        void SetCoturnServerPort(const std::string& port);
        void SetDeviceId(const std::string& id);
        void SetDeviceRandomPwd(const std::string& pwd);
        void SetPanelListeningPort(int port);
        void SetIdServerHost(const std::string& host);
        void SetIdServerPort(const std::string& port);
        void SetRelayServerHost(const std::string& host);
        void SetRelayServerPort(const std::string& port);

        [[nodiscard]] std::string GetCaptureMonitor() const;

    public:
        SharedPreference* sp_ = nullptr;
        std::string version_;
        // @deprecated
        int http_server_port_{0};
        // @deprecated
        int udp_listen_port_{0};

        int panel_listen_port_{0};

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
        std::string capture_audio_device_;

        int network_listening_port_{};
        std::string network_listening_ip_{};
        std::string websocket_enabled_ = kStTrue;
        std::string webrtc_enabled_ = kStTrue;
        std::string udp_kcp_enabled_ = kStTrue;
        // ID Server
        std::string id_server_host_;
        std::string id_server_port_;
        // Relay Server
        std::string relay_server_host_;
        std::string relay_server_port_;
        // Signaling Server
        std::string sig_server_address_;
        std::string sig_server_port_;
        // Coturn Server
        std::string coturn_server_address_;
        std::string coturn_server_port_;

        std::string device_id_;
        std::string device_random_pwd_;

        std::string file_transfer_folder_;

        int sys_service_port_ = 20375;
    };

}

#endif //TC_SERVER_STEAM_GR_SETTINGS_H

//
// Created by RGAA on 2023-08-10.
//

#ifndef SAILFISH_CLIENT_PC_SETTINGS_H
#define SAILFISH_CLIENT_PC_SETTINGS_H

#include <memory>
#include <string>
#include "tc_message.pb.h"

namespace tc
{

    enum class VideoRenderType {
        kOpenGL,
        kTestQPixmap,
    };

    enum class MultiDisplayMode {
        kSeparated,
        kCombined,
    };

    enum class ScaleMode {
        kKeepAspectRatio,
        kFullWindow,
        kOriginSize,
    };

    class SharedPreference;

    class Settings {
    public:

        static Settings* Instance() {
            static Settings sts;
            return &sts;
        }

        void LoadMainSettings();
        void LoadRenderSettings();

        [[nodiscard]] bool IsAudioEnabled() const;
        [[nodiscard]] bool IsClipboardEnabled() const;
        [[nodiscard]] bool IsFullColorEnabled() const;
        MultiDisplayMode GetMultiDisplayMode();
        VideoRenderType GetVideoRenderType();

        // use IP:PORT to connect directly
        [[nodiscard]] bool IsDirectConnect();

        void SetAudioEnabled(bool enabled);
        // 废弃
        void SetTempAudioEnabled(bool enabled);
        void SetClipboardEnabled(bool enabled);
        void SetMultiDisplayMode(MultiDisplayMode mode);
        void SetWorkMode(SwitchWorkMode::WorkMode mode);
        void SetScaleMode(ScaleMode mode);
        void SetFullColorEnabled(bool enabled);
        void SetFps(int fps);
        void Dump();
    public:
        std::string version_;
        bool audio_on_ = false;
        bool clipboard_on_ = false;
        bool full_color_on_ = false;
        MultiDisplayMode display_mode_ = MultiDisplayMode::kSeparated;
        VideoRenderType render_type_ = VideoRenderType::kOpenGL;
        SharedPreference* sp_ = nullptr;
        std::string remote_address_;
        //deprecated
        //int file_transfer_port_ = 20369;
        //std::string file_transfer_path_ = "/file/transfer";
        bool ignore_mouse_event_ = false;
        SwitchWorkMode::WorkMode work_mode_ = SwitchWorkMode::kGame;
        ScaleMode scale_mode_ = ScaleMode::kFullWindow;
        // for client render process --- below
        std::string stream_id_;
        // conn type
        // deprecated !
        // ClientConnectType conn_type_;
        // network type
        ClientNetworkType network_type_;
        // stream name
        std::string stream_name_;
        // device id
        std::string device_id_;
        // device random pwd
        std::string device_random_pwd_;
        // device safety pwd
        std::string device_safety_pwd_;
        // remote device
        std::string remote_device_id_;
        // remote device random pwd
        std::string remote_device_random_pwd_;
        // remote device safety pwd
        std::string remote_device_safety_pwd_;
        // enable p2p
        bool enable_p2p_ = false;
        // show max window
        bool show_max_window_ = false;
        std::string display_name_;
        std::string display_remote_name_;
        // panel ws server port
        int panel_server_port_ = 0;

        //  screen recording path
        std::string screen_recording_path_;

        // fps 当前流路的帧率
        int fps_ = 30;

        // this device host/ip address
        std::string my_host_;

        // language
        int language_ = 3; // default English
    };

}

#endif //SAILFISH_CLIENT_PC_SETTINGS_H

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

        void LoadSettings();
        bool IsAudioEnabled() const;
        bool IsFullColorEnabled() const;
        void SetAudioEnabled(bool enabled);
        void SetClipboardEnabled(bool enabled);
        void SetWorkMode(SwitchWorkMode::WorkMode mode);
        void SetScaleMode(ScaleMode mode);
        void SetFullColorEnabled(bool enabled);
        void SetFps(int fps);
        bool IsRelayMode();
        bool IsDirectMode();
        void Dump();

    public:
        // 1. direct mode
        // host: remote device ip address
        // port: remote device port
        // 2. relay mode
        // host: relay server address
        // port: relay server port
        std::string host_;
        int port_{0};

        std::string version_;
        bool audio_on_ = false;
        bool clipboard_on_ = false;
        bool full_color_on_ = false;
        SharedPreference* sp_ = nullptr;
        SwitchWorkMode::WorkMode work_mode_ = SwitchWorkMode::kGame;
        ScaleMode scale_mode_ = ScaleMode::kFullWindow;
        // for client render process --- below
        std::string stream_id_;
        // network type
        ClientNetworkType network_type_;
        // stream name
        std::string stream_name_;
        // device id
        std::string device_id_;
        // full device id
        // client_xxx_xxx
        std::string full_device_id_;
        // device random pwd
        std::string device_random_pwd_;
        // device safety pwd
        std::string device_safety_pwd_;
        // remote device
        std::string remote_device_id_;
        // full remote device id
        // server_xxx_xxx
        std::string full_remote_device_id_;
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

        // don't send mouse/keyboard events if enabled
        bool only_viewing_ = false;

        // show all windows
        bool split_windows_ = false;

        // max_number_of_screen_window
        int max_number_of_screen_window_ = 2;

        // display logo
        bool display_logo_ = false;

        // develop mode
        bool develop_mode_ = false;
		
		// titlebar color
		int titlebar_color_ = -1;

        std::string appkey_;

        ///////
        ///////
        // from render //
        // file transfer state in render
        bool is_render_file_transfer_enabled_ = true;

        // audio capture enabled in render
        bool is_render_audio_capture_enabled_ = true;

        // can be operated by mouse/keyboard in render
        bool is_render_be_operated_by_mk_ = true;
        ///////
        ///////
    };

}

#endif //SAILFISH_CLIENT_PC_SETTINGS_H

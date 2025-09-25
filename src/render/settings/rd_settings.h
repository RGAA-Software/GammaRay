//
// Created by RGAA on 2023-12-17.
//

#ifndef TC_APPLICATION_SETTINGS_H
#define TC_APPLICATION_SETTINGS_H

#include <map>
#include <string>
#include "tc_steam_manager_new/steam_entities.h"

namespace tc
{

    // description
    struct Description {
        std::string author_;
        std::string version_;
    };

    // encoder
    struct Encoder {

        enum EncoderFormat {
            kH264,
            kHEVC,
        };

        enum EncodeResolutionType {
            kOrigin,
            kSpecify,
        };

        EncoderFormat encoder_format_;
        int fps_ = 60;
        int bitrate_;
        int encode_width_;
        int encode_height_;
        EncodeResolutionType encode_res_type_;
    };

    // capture
    struct Capture {
        enum CaptureAudioType {
            kAudioInner,
            kAudioGlobal,
        };

        enum CaptureVideoType {
            kVideoInner,
            kCaptureScreen,
        };

    public:
        bool IsVideoInnerCapture() const {
            return capture_video_type_ == CaptureVideoType::kVideoInner;
        }

        bool IsAudioInnerCapture() const {
            return capture_audio_type_ == CaptureAudioType::kAudioInner;
        }

    public:
        bool enable_audio_;
        CaptureAudioType capture_audio_type_;
        bool enable_video_;
        CaptureVideoType capture_video_type_;
        bool send_video_frame_by_shm_;
        std::string capture_audio_device_;
        bool mock_video_ = false;
    };

    // Transmission
    struct Transmission {
        int listening_port_ = 0;
        int udp_listen_port_ = 0;
    };

    // RdApplication
    struct TargetApplication {

        enum InjectMethod {
            kEasyHook,
            kOBS,
        };

        enum EventReplayMode {
            kGlobal,
            kHookInner,
        };

        [[nodiscard]] bool IsGlobalReplayMode() const {
            return event_replay_mode_ == EventReplayMode::kGlobal;
        }

    public:
        std::string game_path_{};
        std::string game_arguments_{};
        bool hide_after_started_{};
        bool force_fullscreen_{};
        InjectMethod inject_method_{kEasyHook};
        SteamApp steam_app_;
        bool debug_enabled_{false};
        EventReplayMode event_replay_mode_;

    public:
        [[nodiscard]] bool IsSteamUrl() const {
            return game_path_.find("steam://") != std::string::npos;
        }
    };

    // app mode
    enum class AppMode {
        kDesktop,
        kInnerCapture,
    };

    class RdSettings {
    public:

        static RdSettings* Instance() {
            static RdSettings inst;
            return &inst;
        }

        bool LoadSettings(const std::string& path);
        std::string Dump();
        void LoadSettingsFromDatabase();
        bool EnableFullColorMode();
        void SetFullColorMode(bool enable);
    public:
        Description desc_;
        Encoder encoder_{};
        Capture capture_{};
        Transmission transmission_{};
        TargetApplication app_;

        bool block_debug_ = false;
        int panel_server_port_ = 0;
        std::string device_id_;
        std::string device_random_pwd_;
        std::string device_safety_pwd_;
        std::string relay_host_;
        std::string relay_port_;
        // can be operated
        bool can_be_operated_ = true;
        // relay enabled
        bool relay_enabled_ = true;
        // language
        int language_ = 1;
        // file transfer enabled
        bool file_transfer_enabled_ = true;
        // audio enabled
        bool audio_enabled_ = true;
        // app mode
        AppMode app_mode_ = AppMode::kDesktop;
        // appkey
        std::string appkey_;
        // ethernet
        uint64_t max_transmit_speed_ = 0;
        uint64_t max_receive_speed_ = 0;

    private:
        const std::string kFullColorModeKey = "enable_full_color_mode";
        // 是否启用全彩模式: 如果启用全彩模式, 则编码输出的帧可以解码为yuv444, 否则为yuv420
        bool enable_full_color_mode_ = false;
    };

}

#endif //TC_APPLICATION_SETTINGS_H

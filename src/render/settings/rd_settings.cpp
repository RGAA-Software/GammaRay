//
// Created by RGAA on 2023-12-17.
//

#include "rd_settings.h"

#include <sstream>

#include "toml/toml.hpp"
#include "tc_common_new/string_util.h"
#include "tc_common_new/log.h"
#include "tc_common_new/shared_preference.h"

namespace tc
{

    bool RdSettings::LoadSettings(const std::string& path) {
        toml::parse_result result;
        try {
            result = toml::parse_file(path);
        } catch (std::exception& e) {
            return false;
        }

        // description
        desc_.author_ = result["description"]["author"].value_or("");
        desc_.version_ = result["description"]["version"].value_or("0.0.1");

        // encoder
//        auto encoder_select_type = result["encoder"]["select-type"].value_or("auto");
//        encoder_.encoder_select_type_ = [&]() -> ECreateEncoderPolicy {
//            if (std::string(encoder_select_type) == std::string("auto")) {
//                return ECreateEncoderPolicy::kAuto;
//            }
//            else if (std::string(encoder_select_type) == std::string("specify")) {
//                return ECreateEncoderPolicy::kSpecify;
//            }
//            else {
//                return ECreateEncoderPolicy::kAuto;
//            }
//        }();
//
//        auto encoder_name = result["encoder"]["name"].value_or("nvenc");
//        encoder_.encoder_name_ = [&]() {
//            if (std::string(encoder_name) == std::string("nvenc")) {
//                return ECreateEncoderName::kNVENC;
//            }
//            else if (std::string(encoder_name) == std::string("amf")) {
//                return ECreateEncoderName::kAMF;
//            }
//            else {
//                return ECreateEncoderName::kFFmpeg;
//            }
//        } ();

        auto encoder_format = result["encoder"]["format"].value_or("h264");
        encoder_.encoder_format_ = [&]() {
            if (std::string(encoder_format) == std::string("h264")) {
                return Encoder::EncoderFormat::kH264;
            }
            else if (std::string(encoder_format) == std::string("hevc")) {
                return Encoder::EncoderFormat::kHEVC;
            }
            else {
                return Encoder::EncoderFormat::kH264;
            }
        } ();

        encoder_.bitrate_ = result["encoder"]["bitrate"].value_or(6);

        if (std::string("origin") == result["encoder"]["encode-resolution-type"].value_or("origin")) {
            encoder_.encode_res_type_ = Encoder::EncodeResolutionType::kOrigin;
        } else {
            encoder_.encode_res_type_ = Encoder::EncodeResolutionType::kSpecify;
        }
        encoder_.encode_width_ = result["encoder"]["encode-width"].value_or(1280);
        encoder_.encode_height_ = result["encoder"]["encode-height"].value_or(720);

        // capture
        capture_.enable_audio_ = result["capture"]["enable-audio"].value_or(true);
        std::string capture_audio_type_name = result["capture"]["audio-capture-type"].value_or("global");
        capture_.capture_audio_type_ = [&]() -> Capture::CaptureAudioType {
            if (capture_audio_type_name == "hook") {
                return Capture::CaptureAudioType::kAudioHook;
            }
            else {
                return Capture::CaptureAudioType::kAudioGlobal;
            }
        }();

        capture_.enable_video_ = result["capture"]["enable-video"].value_or(true);
        std::string capture_video_type_name = result["capture"]["video-capture-type"].value_or("hook");
        capture_.capture_video_type_ = [&]() -> Capture::CaptureVideoType {
            if (capture_video_type_name == "hook") {
                return Capture::CaptureVideoType::kVideoHook;
            }
            else {
                return Capture::CaptureVideoType::kCaptureScreen;
            }
        }();

        capture_.send_video_frame_by_shm_ = result["capture"]["send-video-msg-by-shm"].value_or(false);

        // transmission
        transmission_.listening_port_ = result["transmission"]["listening-port"].value_or(20371);

        // TargetApplication
        app_.game_path_ = result["application"]["game-path"].value_or("");
        app_.game_arguments_ = result["application"]["game-arguments"].value_or("");
        app_.hide_after_started_ = result["application"]["hide-after-started"].value_or(false);
        app_.force_fullscreen_ = result["application"]["force-fullscreen"].value_or(false);
        auto inject_method = result["application"]["capture-method"].value_or("obs");
        app_.inject_method_ = [&]() -> TargetApplication::InjectMethod {
            return std::string(inject_method) == "prepare"
                ? TargetApplication::InjectMethod::kEasyHook : TargetApplication::InjectMethod::kOBS;
        }();
        if (app_.IsSteamUrl()) {
            std::vector<std::string> split_value;
            StringUtil::Split(app_.game_path_, split_value, "/");
            if (!split_value.empty()) {
                auto id = std::atoi(split_value[split_value.size()-1].c_str());
                app_.steam_app_.app_id_ = id;
            }
            app_.steam_app_.steam_url_ = app_.game_path_;
        }
        app_.debug_enabled_ = result["application"]["debug-enabled"].value_or(false);
        app_.event_replay_mode_ = std::string("global") == result["application"]["event-replay-mode"].value_or("global")
                                  ? TargetApplication::EventReplayMode::kGlobal : TargetApplication::EventReplayMode::kHookInner;
        return true;
    }

//    uint32_t RdSettings::GetShmBufferSize() const {
//        auto frame_buffer_size = 1920 * 1080 * 4 ;
//        auto default_buffer_size = kHostToClientShmSize;//2 * 1024 * 1024;
//        auto shm_size = (this->encoder_.encoder_select_type_ == ECreateEncoderPolicy::kSpecify && this->encoder_.encoder_name_ == ECreateEncoderName::kFFmpeg) ? frame_buffer_size : default_buffer_size;
//        return shm_size;
//    }

    std::string RdSettings::Dump() {
        std::stringstream ss;
        ss << "Description: \n";
        ss << "  - author: " << desc_.author_ << std::endl;
        ss << "  - version: " << desc_.version_ << std::endl;
        ss << "Encoder: \n";
//        ss << "  - select type: " << (int)encoder_.encoder_select_type_ << " (0 => auto, 1 => specify)" << std::endl;
//        ss << "  - encoder name: " << (int)encoder_.encoder_name_ << " (0=> Unknown, 1 => NVENC, 2 => AMF, 3 => FFmpeg)" << std::endl;
        ss << "  - encoder format: " << encoder_.encoder_format_ << " (0 => H264, 1 => HEVC)" << std::endl;
        ss << "  - bitrate: " << encoder_.bitrate_ << std::endl;
        ss << "  - encode resolution type: " << (int)encoder_.encode_res_type_ << " (0 => origin, 1=> specify) " <<  std::endl;
        ss << "  - encode fps: " << encoder_.fps_ << std::endl;
        ss << "  - encode width: " << encoder_.encode_width_ << ", height: " << encoder_.encode_height_ << std::endl;
        ss << "Capture: \n";
        ss << "  - enable audio: " << capture_.enable_audio_ << std::endl;
        ss << "  - capture audio type: " << capture_.capture_audio_type_ << " (0 => Hook, 1 => Global) " << std::endl;
        ss << "  - enable audio: " << capture_.enable_video_ << std::endl;
        ss << "  - capture video type: " << capture_.capture_video_type_ << " (0 => Hook 1 => Primary Screen) " << std::endl;
        ss << "Transmission: \n";
        ss << "  - listening port: " << transmission_.listening_port_ << std::endl;
        ss << "RdApplication: \n";
        ss << "  - game path: " << app_.game_path_ << std::endl;
        ss << "  - game arguments: " << app_.game_arguments_ << std::endl;
        ss << "  - steam app:" << std::endl;
        ss << "    - app id: " << app_.steam_app_.app_id_ << std::endl;
        ss << "    - steam url: " << app_.steam_app_.steam_url_ << std::endl;
        ss << "  - hide after started: " << app_.hide_after_started_ << std::endl;
        ss << "  - force fullscreen: " << app_.force_fullscreen_ << std::endl;
        ss << "  - event relay mode: " << app_.event_replay_mode_ << std::endl;
        return ss.str();
    }


    void RdSettings::LoadSettingsFromDatabase() {
        auto sp = SharedPreference::Instance();
        enable_full_color_mode_ = sp->GetInt(kFullColorModeKey, 0);
    }

    bool RdSettings::EnableFullColorMode() {
        return enable_full_color_mode_;
    }

    void RdSettings::SetFullColorMode(bool enable) {
        enable_full_color_mode_ = enable;
        auto sp = SharedPreference::Instance();
        sp->PutInt(kFullColorModeKey, enable ? 1 : 0);
    }
}
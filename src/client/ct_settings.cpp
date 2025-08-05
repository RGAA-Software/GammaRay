//
// Created by RGAA on 2023-08-10.
//

#include "client/ct_settings.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/hardware.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/log.h"
#include "version_config.h"

namespace tc
{

    const std::string kKeyInit = "key_init";
    const std::string kKeyAudioStatus = "key_audio_status";
    const std::string kKeyClipboardStatus = "key_clipboard_status";
    const std::string kKeyWorkMode = "key_work_mode";
    const std::string kKeyScaleMode = "key_scale_mode";
    const std::string kKeyDeviceId = "key_device_id";

//    void Settings::LoadMainSettings() {
//        version_ = std::format("V {}", PROJECT_VERSION);
//        sp_ = SharedPreference::Instance();
//        auto init = sp_->Get(kKeyInit);
//        if (init.empty()) {
//            sp_->Put(kKeyInit, "inited");
//            sp_->Put(kKeyAudioStatus, std::to_string(audio_on_));
//            sp_->Put(kKeyClipboardStatus, std::to_string(clipboard_on_));
//            sp_->Put(kKeyWorkMode, std::to_string((int)work_mode_));
//        } else {
//            audio_on_ = std::atoi(sp_->Get(kKeyAudioStatus).c_str());
//            clipboard_on_ = std::atoi(sp_->Get(kKeyClipboardStatus).c_str());
//        }
//
//        device_id_ = sp_->Get(kKeyDeviceId, "");
//    }

    void Settings::LoadSettings() {
        version_ = std::format("V {}", PROJECT_VERSION);
        sp_ = SharedPreference::Instance();
        auto work_mode = sp_->Get(kKeyWorkMode);
        if (!work_mode.empty()) {
            work_mode_ = (SwitchWorkMode::WorkMode)std::atoi(work_mode.c_str());
        }

        auto scale_mode = sp_->Get(kKeyScaleMode);
        if (!scale_mode.empty()) {
            scale_mode_ = (ScaleMode)std::atoi(scale_mode.c_str());
        }
    }

    bool Settings::IsAudioEnabled() const {
        return audio_on_;
    }

    bool Settings::IsFullColorEnabled() const {
        return full_color_on_;
    }

    void Settings::SetAudioEnabled(bool enabled) {
        audio_on_ = enabled;
        sp_->Put(kKeyAudioStatus, std::to_string(enabled));
    }

    void Settings::SetClipboardEnabled(bool enabled) {
        clipboard_on_ = enabled;
        sp_->Put(kKeyClipboardStatus, std::to_string(enabled));
    }

    void Settings::SetFullColorEnabled(bool enabled) {
        full_color_on_ = enabled;
    }

    void Settings::SetWorkMode(SwitchWorkMode::WorkMode mode) {
        work_mode_ = mode;
        sp_->Put(kKeyWorkMode, std::to_string((int)mode));
    }

    void Settings::SetScaleMode(ScaleMode mode) {
        scale_mode_ = mode;
        sp_->Put(kKeyScaleMode, std::to_string((int)mode));
    }

    void Settings::SetFps(int fps) {
        fps_ = fps;
    }

    bool Settings::IsRelayMode() {
        return network_type_ == ClientNetworkType::kRelay;
    }

    bool Settings::IsDirectMode() {
        return network_type_ == ClientNetworkType::kWebsocket;
    }

    void Settings::Dump() {
        LOGI("device id: {}", device_id_);
        LOGI("stream id: {}", stream_id_);
    }

}
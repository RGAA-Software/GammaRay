//
// Created by RGAA on 2023-08-10.
//

#include "Settings.h"

#include "tc_common_new/shared_preference.h"

namespace tc
{

    const std::string kKeyInit = "key_init";
    const std::string kKeyDisplayMode = "key_display_mode";
    const std::string kKeyAudioStatus = "key_audio_status";
    const std::string kKeyClipboardStatus = "key_clipboard_status";
    const std::string kKeyWorkMode = "key_work_mode";
    const std::string kKeyScaleMode = "key_scale_mode";

    void Settings::SetSharedPreference(const std::shared_ptr<SharedPreference>& sp) {
        version_ = "V 1.1.9";
        sp_ = sp;
    }

    void Settings::LoadMainSettings() {
        auto init = sp_->Get(kKeyInit);
        if (init.empty()) {
            sp_->Put(kKeyInit, "inited");
            sp_->Put(kKeyAudioStatus, std::to_string(audio_on_));
            sp_->Put(kKeyClipboardStatus, std::to_string(clipboard_on_));
            sp_->Put(kKeyDisplayMode, std::to_string((int)display_mode_));
            sp_->Put(kKeyWorkMode, std::to_string((int)work_mode_));
        } else {
            audio_on_ = std::atoi(sp_->Get(kKeyAudioStatus).c_str());
            clipboard_on_ = std::atoi(sp_->Get(kKeyClipboardStatus).c_str());
            display_mode_ = (MultiDisplayMode)std::atoi(sp_->Get(kKeyDisplayMode).c_str());
        }
    }

    void Settings::LoadRenderSettings() {
        auto work_mode = sp_->Get(kKeyWorkMode);
        if (!work_mode.empty()) {
            work_mode_ = (SwitchWorkMode::WorkMode)std::atoi(work_mode.c_str());
        }

        auto scale_mode = sp_->Get(kKeyScaleMode);
        if (!scale_mode.empty()) {
            scale_mode_ = (ScaleMode)std::atoi(scale_mode.c_str());
        }
    }

    VideoRenderType Settings::GetVideoRenderType() {
        return render_type_;
    }

    bool Settings::IsAudioEnabled() const {
        return audio_on_;
    }

    bool Settings::IsClipboardEnabled() const {
        return clipboard_on_;
    }

    MultiDisplayMode Settings::GetMultiDisplayMode() {
        return display_mode_;
    }

    void Settings::SetAudioEnabled(bool enabled) {
        audio_on_ = enabled;
        sp_->Put(kKeyAudioStatus, std::to_string(enabled));
    }

    void Settings::SetTempAudioEnabled(bool enabled) {
        audio_on_ = enabled;
    }

    void Settings::SetClipboardEnabled(bool enabled) {
        clipboard_on_ = enabled;
        sp_->Put(kKeyClipboardStatus, std::to_string(enabled));
    }

    void Settings::SetMultiDisplayMode(MultiDisplayMode mode) {
        display_mode_ = mode;
        sp_->Put(kKeyDisplayMode, std::to_string((int)mode));
    }

    void Settings::SetWorkMode(SwitchWorkMode::WorkMode mode) {
        work_mode_ = mode;
        sp_->Put(kKeyWorkMode, std::to_string((int)mode));
    }

    void Settings::SetScaleMode(ScaleMode mode) {
        scale_mode_ = mode;
        sp_->Put(kKeyScaleMode, std::to_string((int)mode));
    }

}
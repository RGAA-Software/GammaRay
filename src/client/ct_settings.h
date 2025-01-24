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
        kSDL,
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

        void SetSharedPreference(const std::shared_ptr<SharedPreference>& sp);
        void LoadMainSettings();
        void LoadRenderSettings();

        bool IsAudioEnabled() const;
        bool IsClipboardEnabled() const;
        MultiDisplayMode GetMultiDisplayMode();
        VideoRenderType GetVideoRenderType();

        void SetAudioEnabled(bool enabled);
        void SetTempAudioEnabled(bool enabled);
        void SetClipboardEnabled(bool enabled);
        void SetMultiDisplayMode(MultiDisplayMode mode);
        void SetWorkMode(SwitchWorkMode::WorkMode mode);
        void SetScaleMode(ScaleMode mode);

    public:
        std::string version_;
        bool audio_on_ = false;
        bool clipboard_on_ = false;
        MultiDisplayMode display_mode_ = MultiDisplayMode::kSeparated;
        VideoRenderType render_type_ = VideoRenderType::kOpenGL;
        std::shared_ptr<SharedPreference> sp_ = nullptr;
        std::string remote_address_;
        int file_transfer_port_ = 20372;
        std::string file_transfer_path_ = "/file/transfer";
        bool ignore_mouse_event_ = false;
        SwitchWorkMode::WorkMode work_mode_ = SwitchWorkMode::kGame;
        ScaleMode scale_mode_ = ScaleMode::kFullWindow;

    };

}

#endif //SAILFISH_CLIENT_PC_SETTINGS_H

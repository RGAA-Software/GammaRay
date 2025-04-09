//
// Created by RGAA on 2023-08-17.
//

#ifndef SAILFISH_CLIENT_PC_APPMESSAGE_H
#define SAILFISH_CLIENT_PC_APPMESSAGE_H

#include <utility>
#include <memory>
#include <string>

#include "tc_message.pb.h"
#include "client/ct_settings.h"

namespace tc
{

    constexpr int kCodeTimeout1S = 0x1001;
    constexpr int kCodeStreamAdded = 0x1002;
    constexpr int kCodeCloseWorkspace = 0x1003;
    constexpr int kCodeClearWorkspace = 0x1004;
    constexpr int kCodeClipboard = 0x1005;
    constexpr int kCodeStreamUpdated = 0x1006;
    constexpr int kCodeMousePressed = 0x1007;
    constexpr int kCodeFullscreen = 0x1008;
    constexpr int kCodeExitFullscreen = 0x1009;
    constexpr int kCodeClipboardStatus = 0x1010;
    constexpr int kCodeAudioStatus = 0x1011;

    /////
    constexpr int kCodeExit = -1;

    const std::string kCaptureAllMonitorsSign = "all";

    // 多显示器展示方式
    enum class EMultiMonDisplayMode {
        kTab,        //标签页
        kSeparate,   //分屏展示
    };

    class AppMessage {
    public:

    };

    // Clipboard
    class ClipboardMessage : public AppMessage {
    public:
        int type_;
        std::string msg_;
        std::vector<ClipboardFile> files_;
    };

    // Mouse pressed
    class MousePressedMessage : public AppMessage {
    public:
        int global_x_;
        int global_y_;
    };

    // Fullscreen
    class FullscreenMessage : public AppMessage {
    public:
    };

    // Exit fullscreen
    class ExitFullscreenMessage : public AppMessage {
    public:
    };

    // Change clipboard status
    class ClipboardStatusMessage : public AppMessage {
    public:
        bool on_ = false;
    };

    // Change audio status
    class AudioStatusMessage : public AppMessage {
    public:
        bool on_ = false;
    };

    class ExitAppMessage : public AppMessage {

    };

    class CaptureMonitorMessage : public AppMessage {
    public:
        class Resolution {
        public:
            int width_ = 0;
            int height_ = 0;
        };

        class CaptureMonitor {
        public:
            bool IsValid() {
                return !name_.empty();
            }
        public:
            std::string name_;
            std::vector<Resolution> resolutions_;
        };

        CaptureMonitor GetCaptureMonitorByName(const std::string& name) {
            for (auto& m : monitors_) {
                if (m.name_ == name) {
                    return m;
                }
            }
            return {};
        }

    public:
        std::string capturing_monitor_name_;
        std::vector<CaptureMonitor> monitors_;
    };

    class SwitchMonitorMessage : public AppMessage {
    public:
        std::string name_;
    };

    class MonitorSwitchedMessage : public AppMessage {
    public:
        std::string name_;
        int index_ = 0;
    };

    class SwitchWorkModeMessage : public AppMessage {
    public:
        SwitchWorkMode::WorkMode mode_;
    };

    class SwitchScaleModeMessage : public AppMessage {
    public:
        ScaleMode mode_;
    };

    // change monitor resolution
    class MsgChangeMonitorResolution {
    public:
        std::string monitor_name_;
        int width_ = 0;
        int height_ = 0;
    };

    // monitor changed from video frame callback
    class MsgMonitorChanged : public AppMessage {
    public:
    };

    // request control + alt + delete
    class MsgCtrlAltDelete : public AppMessage {
    public:
    };

    // MultiMonDisplayMode
    class MultiMonDisplayModeMessage : public AppMessage {
    public:
        EMultiMonDisplayMode mode_;
        int current_cap_mon_index_ = 0;
    };
}

#endif //SAILFISH_CLIENT_PC_APPMESSAGE_H

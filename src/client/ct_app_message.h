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

    const std::string kCaptureAllMonitorsSign = "all";

    const int kMainGameViewIndex = 0;

    // 多显示器展示方式
    enum class EMultiMonDisplayMode {
        kTab,        //标签页
        kSeparate,   //分屏展示
    };

    class MsgClientBase {
    public:

    };

    // Clipboard
    class MsgClientClipboard : public MsgClientBase {
    public:
        int type_;
        std::string msg_;
        std::vector<ClipboardFile> files_;
    };

    // Mouse pressed
    class MsgClientMousePressed : public MsgClientBase {
    public:
        int global_x_;
        int global_y_;
    };

    // Fullscreen
    class MsgClientFullscreen : public MsgClientBase {
    public:
    };

    // Exit fullscreen
    class MsgClientExitFullscreen : public MsgClientBase {
    public:
    };

    class MsgClientExitApp : public MsgClientBase {

    };

    class MsgClientCaptureMonitor : public MsgClientBase {
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

    class MsgClientSwitchMonitor : public MsgClientBase {
    public:
        std::string name_;
    };

    class MsgClientMonitorSwitched : public MsgClientBase {
    public:
        std::string name_;
        int index_ = 0;
    };

    class MsgClientSwitchWorkMode : public MsgClientBase {
    public:
        SwitchWorkMode::WorkMode mode_;
    };

    // 修改帧率 client->render
    class MsgClientModifyFps : public MsgClientBase {
    public:
        int fps_ = 30;
    };

    // 切换全彩模式
    class MsgClientSwitchFullColor : public MsgClientBase {
    public:
        bool enable_ = false;
    };

    class MsgClientSwitchScaleMode : public MsgClientBase {
    public:
        ScaleMode mode_;
    };

    // change monitor resolution
    class MsgClientChangeMonitorResolution {
    public:
        std::string monitor_name_;
        int width_ = 0;
        int height_ = 0;
    };

    // monitor changed from video frame callback
    class MsgClientMonitorChanged : public MsgClientBase {
    public:
    };

    // request control + alt + delete
    class MsgClientCtrlAltDelete : public MsgClientBase {
    public:
    };

    // request update desktop
    class MsgClientHardUpdateDesktop : public MsgClientBase {
    public:
    };

    // MultiMonDisplayMode
    class MsgClientMultiMonDisplayMode : public MsgClientBase {
    public:
        EMultiMonDisplayMode mode_;
        int current_cap_mon_index_ = 0;
    };

    class MsgClientFloatControllerPanelUpdate : public MsgClientBase {
    public:
        enum class EUpdate {
            kUnknow,
            kImageScaleMode,
            kWorkMode,
            kClipboardSharedStatus,
            kAudioStatus,
            kFullColorStatus,
            kMediaRecordStatus,
            kFps,
        };
        EUpdate update_type_ = EUpdate::kUnknow;
    };

    class MsgClientOpenFiletrans : public MsgClientBase {
    public:
    };

    class MsgClientOpenDebugPanel : public MsgClientBase {
    public:
    };

    // clipboard updated
    class MsgClientClipboardUpdated : public MsgClientBase {
    public:
#ifdef WIN32
        HWND hwnd_ = nullptr;
#endif
    };

    class MsgClientMediaRecord : public MsgClientBase {
    public:
    };

    class MsgClientMouseEnterView : public MsgClientBase {
    public:
    };

    class MsgClientMouseLeaveView : public MsgClientBase {
    public:
    };
}

#endif //SAILFISH_CLIENT_PC_APPMESSAGE_H

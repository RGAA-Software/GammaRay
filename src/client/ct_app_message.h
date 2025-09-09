//
// Created by RGAA on 2023-08-17.
//

#ifndef SAILFISH_CLIENT_PC_APPMESSAGE_H
#define SAILFISH_CLIENT_PC_APPMESSAGE_H

#include <utility>
#include <memory>
#include <string>
#include <QVariantMap>

#include "tc_message.pb.h"
#include "client/ct_settings.h"
#include "notify/notify_defs.h"

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
            int current_width_ = 0;
            int current_height_ = 0;
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

    class MsgClientMediaRecord : public MsgClientBase {
    public:
    };

    class MsgClientMouseEnterView : public MsgClientBase {
    public:
    };

    class MsgClientMouseLeaveView : public MsgClientBase {
    public:
    };

    class MsgExitControlledEndExe : public MsgClientBase {
    public:
    };

    // file transmission begins
    class MsgClientFileTransmissionBegin {
    public:
        std::string the_file_id_;
        int64_t begin_timestamp_;
        std::string direction_;
        std::string file_detail_;
        std::string remote_device_id_;
    };

    // file transmission ends
    class MsgClientFileTransmissionEnd {
    public:
        std::string the_file_id_;
        int64_t end_timestamp_;
        int64_t duration_;
        bool success_;
    };

    // notification
    class MsgClientNotificationClicked {
    public:
        NotifyItem data_;
    };

    class MsgClientHidePanel : public MsgClientBase {
    public:
    };

    class MsgClientFocusOutEvent : public MsgClientBase {
    public:
    };

    class MsgRestartRender : public MsgClientBase {
    public:
    };
}

#endif //SAILFISH_CLIENT_PC_APPMESSAGE_H

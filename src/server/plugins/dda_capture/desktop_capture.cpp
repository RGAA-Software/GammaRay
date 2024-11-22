//
// Created by RGAA  on 2024/1/18.
//

#include "desktop_capture.h"
//#include "capture_message.h"
#include "tc_common_new/log.h"
//#include "tc_common_new/message_notifier.h"
#include <Shlobj.h>

namespace tc
{

    DesktopCapture::DesktopCapture(DDACapturePlugin* plugin, const std::string& monitor) {
        plugin_ = plugin;
//        msg_notifier_ = msg_notifier;
        capturing_monitor_name_ = monitor;
//        msg_listener_ = msg_notifier->CreateListener();
//        msg_listener_->Listen<RefreshScreenMessage>([this](const RefreshScreenMessage& msg) {
//            this->RefreshScreen();
//            LOGI("Refresh screen.");
//        });
    }

    void DesktopCapture::SetCaptureMonitor(int index, const std::string& name) {
        std::lock_guard<std::mutex> lk(capturing_monitor_mtx_);
        capturing_monitor_index_ = index;
        capturing_monitor_name_ = name;
        refresh_screen_ = true;
    }

    void DesktopCapture::SetCaptureFps(int fps) {
        capture_fps_ = fps;
    }

    std::vector<CaptureMonitorInfo> DesktopCapture::GetCaptureMonitorInfo() {
        return sorted_monitors_;
    }

    void DesktopCapture::RefreshScreen() {
        SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, nullptr, SPIF_SENDCHANGE);
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
        refresh_screen_ = true;
    }

    void DesktopCapture::SendCapturingMonitorMessage() {
//        msg_notifier_->SendAppMessage(CaptureMonitorInfoMessage {
//            .monitors_ = sorted_monitors_,
//            .capturing_monitor_name_ = this->capturing_monitor_name_,
//        });
    }

    int DesktopCapture::GetCapturingMonitorIndex() const {
        return capturing_monitor_index_;
    }

    std::string DesktopCapture::GetCapturingMonitorName() {
        return capturing_monitor_name_;
    }

}
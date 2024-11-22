//
// Created by RGAA  on 2024/1/18.
//

#ifndef TC_APPLICATION_DESKTOP_CAPTURE_H
#define TC_APPLICATION_DESKTOP_CAPTURE_H

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include "tc_capture_new/win/desktop_capture/monitor_util.h"

namespace tc
{
    class DDACapturePlugin;

    class DesktopCapture {
    public:
        explicit DesktopCapture(DDACapturePlugin* plugin, const std::string& monitor);
        virtual bool Init() = 0;
        virtual bool StartCapture() = 0;
        virtual void StopCapture() = 0;
        void SetCaptureMonitor(int index, const std::string& name);
        void SetCaptureFps(int fps);
        std::vector<CaptureMonitorInfo> GetCaptureMonitorInfo();
        void SendCapturingMonitorMessage();
        [[nodiscard]] int GetCapturingMonitorIndex() const;
        std::string GetCapturingMonitorName();

    private:
        void RefreshScreen();

    protected:
        DDACapturePlugin* plugin_ = nullptr;
        std::mutex capturing_monitor_mtx_;
        std::string capturing_monitor_name_;
        int capturing_monitor_index_ = 0;
        int capture_fps_ = 60;
        std::vector<CaptureMonitorInfo> sorted_monitors_;
        bool refresh_screen_ = false;
    };
}

#endif //TC_APPLICATION_DESKTOP_CAPTURE_H

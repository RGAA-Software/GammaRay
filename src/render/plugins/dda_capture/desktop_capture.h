//
// Created by RGAA  on 2024/1/18.
//

#ifndef TC_APPLICATION_DESKTOP_CAPTURE_H
#define TC_APPLICATION_DESKTOP_CAPTURE_H

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <deque>
#include "tc_capture_new/win/desktop_capture/monitor_util.h"

namespace tc
{
    class DDACapturePlugin;

    class DesktopCapture {
    public:
        explicit DesktopCapture(DDACapturePlugin* plugin, const CaptureMonitorInfo& my_monitor_info);
        virtual bool Init() = 0;
        virtual bool StartCapture() = 0;
        virtual bool PauseCapture() = 0;
        virtual void ResumeCapture() = 0;
        virtual bool IsPausing() { return pausing_; }
        virtual void StopCapture() = 0;
        // set the value that our expected
        virtual void SetCaptureFps(int fps);
        virtual void RefreshScreen();
        virtual bool IsPrimaryMonitor();
        virtual bool IsInitSuccess() = 0;
        // by counting the real frames
        virtual int GetCapturingFps() {return 0;}
        virtual std::vector<int32_t> GetCaptureGaps();
        virtual CaptureMonitorInfo GetMyMonitorInfo() { return my_monitor_info_; }
    private:

    protected:
        DDACapturePlugin* plugin_ = nullptr;
        int capture_fps_ = 60;
        bool refresh_screen_ = false;
        std::atomic_bool pausing_ = true;
        CaptureMonitorInfo my_monitor_info_;
        bool is_primary_monitor_ = false;
        std::deque<int32_t> capture_gaps_;
    };
}

#endif //TC_APPLICATION_DESKTOP_CAPTURE_H

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <deque>
#include "tc_common_new/concurrent_queue.h"
#include "hook_capture/win/desktop_capture/monitor_util.h"

namespace tc
{

    class PluginDesktopCapture {
    public:
        explicit PluginDesktopCapture(const CaptureMonitorInfo& my_monitor_info);
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
        virtual void TryWakeOs();

    protected:
        int capture_fps_ = 60;
        bool refresh_screen_ = false;
        std::atomic_bool pausing_ = false;
        CaptureMonitorInfo my_monitor_info_;
        bool is_primary_monitor_ = false;
        ConcurrentQueue<int32_t> capture_gaps_;
    };
}


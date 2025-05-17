#include "plugin_desktop_capture.h"
#include "tc_common_new/log.h"
#include "plugin_interface/gr_plugin_events.h"
#include <Shlobj.h>
//#include "dda_capture_plugin.h"

namespace tc
{

    PluginDesktopCapture::PluginDesktopCapture(/*DDACapturePlugin* plugin,*/ const CaptureMonitorInfo& my_monitor_info) {
        //plugin_ = plugin;
        my_monitor_info_ = my_monitor_info;
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    }

    void PluginDesktopCapture::SetCaptureFps(int fps) {
        capture_fps_ = fps;
    }

    void PluginDesktopCapture::RefreshScreen() {
        refresh_screen_ = true;
    }

    bool PluginDesktopCapture::IsPrimaryMonitor() {
        return false;
    }

    std::vector<int32_t> PluginDesktopCapture::GetCaptureGaps() {
        std::vector<int32_t> result;
        for (const auto& item : capture_gaps_) {
            result.push_back(item);
        }
        return result;
    }

}
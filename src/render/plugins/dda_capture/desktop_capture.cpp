//
// Created by RGAA  on 2024/1/18.
//

#include "desktop_capture.h"
#include "tc_common_new/log.h"
#include "plugin_interface/gr_plugin_events.h"
#include <Shlobj.h>
#include "dda_capture_plugin.h"

namespace tc
{

    DesktopCapture::DesktopCapture(DDACapturePlugin* plugin, const CaptureMonitorInfo& my_monitor_info) {
        plugin_ = plugin;
        my_monitor_info_ = my_monitor_info;
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    }

    void DesktopCapture::SetCaptureFps(int fps) {
        capture_fps_ = fps;
    }

    void DesktopCapture::RefreshScreen() {
        SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, nullptr, SPIF_SENDCHANGE);
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
        refresh_screen_ = true;
    }

}
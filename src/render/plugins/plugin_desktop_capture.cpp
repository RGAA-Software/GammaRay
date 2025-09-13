#include "plugin_desktop_capture.h"
#include "tc_common_new/log.h"
#include "plugin_interface/gr_plugin_events.h"
#include <Shlobj.h>

namespace tc
{

    PluginDesktopCapture::PluginDesktopCapture(const CaptureMonitorInfo& my_monitor_info) {
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
        return capture_gaps_.ToVector();
    }

    void PluginDesktopCapture::TryWakeOs() {
        // mock 1 pixel mouse move
        {
            INPUT input = {0};
            input.type = INPUT_MOUSE;
            input.mi.dx = 1;
            input.mi.dy = 1;
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(INPUT));
        }
        {
            INPUT input = {0};
            input.type = INPUT_MOUSE;
            input.mi.dx = -1;
            input.mi.dy = -1;
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(INPUT));
        }
        // mock a keyboard event
        //INPUT inputs[2] = {0};
        //inputs[0].type = INPUT_KEYBOARD;
        //inputs[0].ki.wVk = VK_SHIFT;
        //inputs[0].ki.dwFlags = 0;

        //inputs[1].type = INPUT_KEYBOARD;
        //inputs[1].ki.wVk = VK_SHIFT;
        //inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
        //SendInput(2, inputs, sizeof(INPUT));
    }

    void PluginDesktopCapture::On16MilliSecond() {

    }

    void PluginDesktopCapture::On33MilliSecond() {

    }

}
//
// Created by RGAA  on 2024/2/12.
//

#include "win_event_replayer.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"
#include "plugin_interface/gr_monitor_capture_plugin.h"

namespace tc
{
    const uint32_t kExtendedKeys[] = {
        VK_DELETE, VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN, VK_NUMLOCK,
        VK_RCONTROL, VK_RMENU, VK_RETURN, VK_DIVIDE, VK_LWIN,
        VK_RWIN, VK_HOME, VK_PRIOR, VK_NEXT, VK_END, VK_INSERT,
    };

    void WinSendEvent(INPUT* input) {
        if (!input) {
            return;
        }
        bool is_ok = true;
        if (SendInput(1, input, sizeof(INPUT)) != 1) {
            HDESK desk = OpenInputDesktop(0, FALSE, GENERIC_ALL);
            if (desk) {
                if (!SetThreadDesktop(desk)) {
                }
                if (SendInput(1, input, sizeof(INPUT)) == 1) {
                } else {
                    is_ok = false;
                }
                CloseDesktop(desk);
            } else {
                is_ok = false;
            }
        }
        if(!is_ok) {
            LOGE("SendInput error");
        }
    }

    INPUT GenerateScanCodeInput(uint16_t scancode, bool down, bool extend) {
        INPUT evt;
        memset(&evt, 0, sizeof(evt));
        evt.type = INPUT_KEYBOARD;
        evt.ki.wVk = 0;
        evt.ki.dwFlags = KEYEVENTF_SCANCODE;

        if (!down) {
            evt.ki.dwFlags |= KEYEVENTF_KEYUP;
        }
        if (extend) {
            evt.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        }

        evt.ki.wScan = scancode;
        evt.ki.dwExtraInfo = 0;
        evt.ki.time = 0;
        if (evt.ki.wScan == 0x45) {
            evt.ki.dwFlags &= ~KEYEVENTF_SCANCODE;
            if (evt.ki.dwFlags & KEYEVENTF_EXTENDEDKEY) {
                evt.ki.wVk = VK_NUMLOCK;
            } else {
                evt.ki.wVk = VK_PAUSE;
            }
        }
        if (evt.ki.wScan == 0x3a) {
            evt.ki.dwFlags &= ~KEYEVENTF_SCANCODE;
            evt.ki.wVk = VK_CAPITAL;
        }
        return evt;
    }

    void WinEventReplayer::HandleKeyEvent(const tc::KeyEvent& event) {
        bool down = event.down();
        uint32_t vk_code = event.key_code();
        if (vk_code > 255) {
            LOGE("Error vk: {}", vk_code);
            return;
        }

        current_key_status_[vk_code] = down;
        if (!IsKeyPermitted(vk_code)) {
            current_key_status_[vk_code] =!down;
            return;
        }

        if (vk_code == VK_CONTROL || vk_code == VK_RCONTROL || vk_code == VK_LCONTROL) {
            control_pressed_ = down;
        }
        if (vk_code == VK_MENU || vk_code == VK_RMENU || vk_code == VK_LMENU) {
            menu_pressed_ = down;
        }
        if (vk_code == VK_DELETE) {
            delete_pressed_ = down;
        }
        if (vk_code == VK_SHIFT || vk_code == VK_RSHIFT || vk_code == VK_LSHIFT) {
            shift_pressed_ = down;
        }
        if (vk_code == VK_LWIN || vk_code == VK_RWIN) {
            win_pressed_ = down;
        }

        if(control_pressed_ && menu_pressed_ && delete_pressed_ && !shift_pressed_ && !win_pressed_) {
            return;
        }
        UINT vsc = MapVirtualKey(vk_code, MAPVK_VK_TO_VSC);
        bool extend = false;
        for (size_t j = 0; j < sizeof(kExtendedKeys) / sizeof(UINT32); j++) {
            if (kExtendedKeys[j] == vk_code) {
                extend = true;
                break;
            }
        }
        //LOGI("vk code: {}, down: {}, scancode: {}, extend: {}", vk_code, down, vsc, extend);
        ReplayKeyEvent(vsc, extend, event);
    }

    bool WinEventReplayer::IsKeyPermitted(uint32_t vk) {
        return true;
    }

    void WinEventReplayer::ResetKey() {
        for(int i = 0; i < sizeof(current_key_status_) / sizeof(*current_key_status_);++i) {
            if (current_key_status_[i]) {
                tc::KeyEvent event;
                event.set_down(false);
                event.set_key_code(i);
                HandleKeyEvent(event);
            }
        }
    }

    void WinEventReplayer::ReplayKeyEvent(uint16_t scancode, bool extend, const tc::KeyEvent& event) {
        INPUT evt = GenerateScanCodeInput(scancode, event.down(), extend);
        short num_lock_status = event.num_lock_status();
        short curr_num_lock_status = GetKeyState(VK_NUMLOCK);

        if (evt.ki.wVk == VK_NUMLOCK) {
            LOGI("numlock status : {}, current status: {}", num_lock_status, curr_num_lock_status);
            if (curr_num_lock_status != num_lock_status) {
                LOGI("NumLock, NO need to send event.");
                return;
            }
        } else {
            if (event.down() && num_lock_status != curr_num_lock_status && event.status_check() == KeyEvent::kCheckNumLock) {
                MockKeyEvent(0x45);
            }
        }

        short caps_lock_status = event.caps_lock_status();
        short curr_caps_lock_status = GetKeyState(VK_CAPITAL);
        if (scancode == 0x3a) {
            if (curr_caps_lock_status == caps_lock_status) {
                return;
            }
        } else {
            if (event.down() && caps_lock_status != curr_caps_lock_status && event.status_check() == KeyEvent::kCheckCapsLock) {
                MockKeyEvent(0x3a);
            }
        }
        WinSendEvent(&evt);
    }

    void WinEventReplayer::MockKeyEvent(uint16_t scancode) {
        INPUT down = GenerateScanCodeInput(scancode, true, true);
        WinSendEvent(&down);
        INPUT up = GenerateScanCodeInput(scancode, false, true);
        WinSendEvent(&up);
    }

    void WinEventReplayer::HandleMouseEvent(const tc::MouseEvent& event) {
        float x_ratio = event.x_ratio();
        float y_ratio = event.y_ratio();
        std::string monitor_name = event.monitor_name();
        int button = event.button();
        int data = event.data();
        ReplayMouseEvent(monitor_name, x_ratio, y_ratio, button, data);
    }

    void WinEventReplayer::ReplayMouseEvent(const std::string& monitor_name, float x_ratio, float y_ratio, int buttons, int data) {

        //if (kVirtualDesktopNameSign == monitor_name) {
        //    ReplayVirtualDesktopMouseEvent(x_ratio, y_ratio, buttons, data);
        //    return;
        //}

        if (monitors_.empty()) {
            LOGE("Don't have capturing monitor info.");
            return;
        }
        auto func_find_monitor = [&]() -> CaptureMonitorInfo {
            for (auto& mon : monitors_) {
                if (std::string(mon.name_) == monitor_name) {
                    return mon;
                }
            }
            return CaptureMonitorInfo{};
        };

        auto target_monitor = func_find_monitor();
        if (!target_monitor.Valid()) {
            LOGE("Invalid monitor for name: {}", monitor_name);
            return;
        }
        // LOGI("monitor idx: {}, left: {}, bottom: {}, v-left: {}, v-bottom: {}",
        //      monitor_index, target_monitor.left_, target_monitor.bottom_,
        //      target_monitor.virtual_left_, target_monitor.virtual_bottom_);
        int x = (
            x_ratio * target_monitor.Width()
            +
            std::abs(target_monitor.left_ - virtual_desktop_bound_rectangle_info_.far_left_)
            ) * 65535
            /
            (virtual_desktop_bound_rectangle_info_.far_right_ - virtual_desktop_bound_rectangle_info_.far_left_ - 1);

        int y = (
            y_ratio * target_monitor.Height()
            +
            std::abs(target_monitor.top_ - virtual_desktop_bound_rectangle_info_.far_top_)
            ) * 65535
            /
            (virtual_desktop_bound_rectangle_info_.far_bottom_ - virtual_desktop_bound_rectangle_info_.far_top_ - 1);

        
        SendMouseEvent(x, y, buttons, data);
    }

    void WinEventReplayer::UpdateCaptureMonitorInfo(const CaptureMonitorInfoMessage& msg) {
        monitors_ = msg.monitors_;
        virtual_desktop_bound_rectangle_info_ = msg.virtual_desktop_bound_rectangle_info_;
        //LOGI("UpdateCaptureMonitorInfo, monitor count: {}", monitors_.size());
    }

    void WinEventReplayer::ReplayVirtualDesktopMouseEvent(float x_ratio, float y_ratio, int buttons, int data) {
        int x = x_ratio * 65535;
        int y = y_ratio * 65535;
        SendMouseEvent(x, y, buttons, data);
    }

    void WinEventReplayer::SendMouseEvent(int x, int y, int buttons, int data) {
        INPUT evt;
        evt.type = INPUT_MOUSE;
        evt.mi.dx = x;
        evt.mi.dy = y;
        int target_buttons = 0;
        if (buttons & ButtonFlag::kMouseMove) {
            target_buttons |= MOUSEEVENTF_MOVE;
        }
        if (buttons & ButtonFlag::kLeftMouseButtonDown) {
            target_buttons |= MOUSEEVENTF_LEFTDOWN;
        }
        if (buttons & ButtonFlag::kRightMouseButtonDown) {
            target_buttons |= MOUSEEVENTF_RIGHTDOWN;
        }
        if (buttons & ButtonFlag::kMiddleMouseButtonDown) {
            target_buttons |= MOUSEEVENTF_MIDDLEDOWN;
        }
        if (buttons & ButtonFlag::kLeftMouseButtonUp) {
            target_buttons |= MOUSEEVENTF_LEFTUP;
        }
        if (buttons & ButtonFlag::kRightMouseButtonUp) {
            target_buttons |= MOUSEEVENTF_RIGHTUP;
        }
        if (buttons & ButtonFlag::kMiddleMouseButtonUp) {
            target_buttons |= MOUSEEVENTF_MIDDLEUP;
        }
        if (buttons & ButtonFlag::kMouseEventWheel) {
            target_buttons |= MOUSEEVENTF_WHEEL;
        }

        target_buttons |= MOUSEEVENTF_ABSOLUTE;
        target_buttons |= MOUSEEVENTF_VIRTUALDESK;

        evt.mi.dwFlags = target_buttons;
        evt.mi.dwExtraInfo = 0;
        evt.mi.mouseData = data;
        evt.mi.time = 0;
        WinSendEvent(&evt);
    }

    void WinEventReplayer::HandleFocusOutEvent() {

        std::array<int, 11> keys = {
            VK_CONTROL,
            VK_RCONTROL,
            VK_LCONTROL,
            VK_SHIFT,
            VK_RSHIFT,
            VK_LSHIFT,
            VK_MENU,
            VK_RMENU,
            VK_LMENU,
            VK_LWIN,
            VK_RWIN,
        };
        
        for (auto key : keys) {
            tc::KeyEvent event;
            event.set_down(false);
            event.set_key_code(key);
            HandleKeyEvent(event);
        }
    }

    void WinEventReplayer::SimulateCtrlWinShiftB() {
        INPUT inputs[8] = {};
        ZeroMemory(inputs, sizeof(inputs));

        // 按下Ctrl
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_CONTROL;
        inputs[0].ki.dwFlags = 0;

        // 按下Shift
        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = VK_SHIFT;
        inputs[1].ki.dwFlags = 0;

        // 按下Win
        inputs[2].type = INPUT_KEYBOARD;
        inputs[2].ki.wVk = VK_LWIN;
        inputs[2].ki.dwFlags = 0;

        // 按下B
        inputs[3].type = INPUT_KEYBOARD;
        inputs[3].ki.wVk = 'B';
        inputs[3].ki.dwFlags = 0;

        // 释放B
        inputs[4].type = INPUT_KEYBOARD;
        inputs[4].ki.wVk = 'B';
        inputs[4].ki.dwFlags = KEYEVENTF_KEYUP;

        // 释放Win
        inputs[5].type = INPUT_KEYBOARD;
        inputs[5].ki.wVk = VK_LWIN;
        inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;

        // 释放Shift
        inputs[6].type = INPUT_KEYBOARD;
        inputs[6].ki.wVk = VK_SHIFT;
        inputs[6].ki.dwFlags = KEYEVENTF_KEYUP;

        // 释放Ctrl
        inputs[7].type = INPUT_KEYBOARD;
        inputs[7].ki.wVk = VK_CONTROL;
        inputs[7].ki.dwFlags = KEYEVENTF_KEYUP;

        // 发送所有输入事件
        SendInput(8, inputs, sizeof(INPUT));
    }
}
//
// Created by Administrator on 2024/2/12.
//

#include "win_event_replayer.h"
#include <Windows.h>
#include "settings/settings.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"

namespace tc
{
    const UINT32 kExtendedKeys[] = {
            VK_RCONTROL,
            VK_RMENU,
            VK_RETURN,
            VK_DIVIDE,
            VK_LWIN,
            VK_RWIN,
            VK_HOME,
            VK_PRIOR,
            VK_NEXT,
            VK_END,
            VK_INSERT,
            VK_DELETE,
            VK_LEFT,
            VK_UP,
            VK_RIGHT,
            VK_DOWN,
            VK_NUMLOCK
    };

    void WinSendEvent(INPUT* input) {
        if (!input) {
            return;
        }
        bool is_ok = true;
        if (SendInput(1, input, sizeof(INPUT)) != 1)
        {
            //LogW(kLogInput, "SendInput {}", GetLastError());
            HDESK desk = OpenInputDesktop(0, FALSE, GENERIC_ALL);
            if (desk)
            {
                if (!SetThreadDesktop(desk))
                {
                    //LogW(kLogInput, "SetThreadDesktop failed with:{}", GetLastError());
                }
                if (SendInput(1, input, sizeof(INPUT)) == 1)
                {
                    //LogI(kLogInput, "try send input successfully.");
                }
                else
                {
                    is_ok = false;
                    //LogE(kLogInput, "Re SendInput failed {}", GetLastError());
                }
                CloseDesktop(desk);
            }
            else
            {

                is_ok = false;
            }
        }
        if(!is_ok) {
            std::cout << "SendInput error" << std::endl;
        }
        // 事件处理的统计  to do ...
    }

    INPUT GenerateScanCodeInput(uint16_t scancode, bool down, bool extend) {
        INPUT evt;
        memset(&evt, 0, sizeof(evt));
        evt.type = INPUT_KEYBOARD;
        evt.ki.wVk = 0;
        evt.ki.dwFlags = KEYEVENTF_SCANCODE;

        if (!down)
            evt.ki.dwFlags |= KEYEVENTF_KEYUP;

        if (extend) {
            evt.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        }

        evt.ki.wScan = scancode;
        evt.ki.dwExtraInfo = 0;
        evt.ki.time = 0;
        //LOGI("SendInput ScanCode: 0x%x Flags: 0x%lx %s", scancode,
        //	evt.ki.dwFlags, down ? "Down" : "Up");

        // Windows has some bug where it doesn't look up scan code 0x45
        // properly, so we need to help it out
        if (evt.ki.wScan == 0x45) {
            evt.ki.dwFlags &= ~KEYEVENTF_SCANCODE;
            if (evt.ki.dwFlags & KEYEVENTF_EXTENDEDKEY)
                evt.ki.wVk = VK_NUMLOCK;
            else
                evt.ki.wVk = VK_PAUSE;
        }
        if (evt.ki.wScan == 0x3a) {
            evt.ki.dwFlags &= ~KEYEVENTF_SCANCODE;
            evt.ki.wVk = VK_CAPITAL;
        }
        return evt;
    }

    void WinEventReplayer::HandleMessage(const std::shared_ptr<Message>& msg) {
        // to do
        switch (msg->type()) {
            case MessageType::kKeyEvent: {
                auto key_ev = msg->key_event();
                HandleKeyEvent(key_ev);
                break;
            }
            case MessageType::kMouseEvent: {
                auto mouse_ev =  msg->mouse_event();
                HandleMouseEvent(mouse_ev);
                break;
            }
        }
    }

    void WinEventReplayer::HandleKeyEvent(const tc::KeyEvent& event) {
        bool down = event.down();
        uint32_t vk_code = event.key_code();
        if (vk_code > 255)
        {
            std::cout << "vk_code > 255" << std::endl;
            return;
        }

        current_key_status_map[vk_code] = down;
        if (!CheckKeyAllowDown(vk_code, down))
        {
            current_key_status_map[vk_code] =!down;
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

        // to do 是否是全局，还是进程内部响应，暂定 截屏方式时 使用全局响应事件
        if(Settings::Instance()->capture_.capture_video_type_ == Capture::kCaptureScreen) {
            // 是否为 ctrl + delete + alt
            if(control_pressed_ && menu_pressed_ && delete_pressed_ && !shift_pressed_ && !win_pressed_) {
                return;
            }
            // MAPVK_VK_TO_VSC：将虚拟键码转换为扫描码值。
            std::cout << "down =  " << down << "HandleKeyEvent vk_code = " << vk_code << std::endl;
            UINT vsc = MapVirtualKey(vk_code, MAPVK_VK_TO_VSC);
            bool extend = false;
            for (size_t j = 0; j < sizeof(kExtendedKeys) / sizeof(UINT32); j++) {
                if (kExtendedKeys[j] == vk_code) {
                    extend = true;
                    break;
                }
            }
            LOGI("vk code: {}, down: {}, scancode: {}, extend: {}", vk_code, down, vsc, extend);
            DoScanCodeEvent(vsc, extend, event);
        }
    }

    bool WinEventReplayer::CheckKeyAllowDown(uint32_t vk, bool down)
    {
        // win
        // to do
#if 1   // 如果是hook游戏画面, 并且按键是 VK_LWIN 或 VK_RWIN 就不允许按下
        if(Settings::Instance()->capture_.capture_video_type_ == Capture::kVideoHook && (vk == VK_LWIN || vk == VK_RWIN )) {
            return false;
        }
#endif
        // alt + F4
        if(current_key_status_map[VK_MENU] && current_key_status_map[VK_F4]) {
            return false;
        }
        return true;
    }

    void WinEventReplayer::ResetKey()
    {
        // to do 当客户端断开连接时,重置服务端的键盘的状态。
        for(int i = 0; i < sizeof(current_key_status_map) / sizeof(*current_key_status_map);++i)
        {
            if (current_key_status_map[i]) {
                tc::KeyEvent event;
                event.set_down(false);
                event.set_key_code(i);
                HandleKeyEvent(event);
            }
        }
    }

    // 扫描码输入
    void WinEventReplayer::DoScanCodeEvent(uint16_t scancode, bool extend, const tc::KeyEvent& event) {
        INPUT evt = GenerateScanCodeInput(scancode, event.down(), extend);

        short num_lock_status = event.num_lock_status();
        short curr_num_lock_status = GetKeyState(VK_NUMLOCK);

        //当按下num lock时，如果server端的状态跟client不相同，则不需要发送此次事件，
        //因为num lock有两种状态，经过测试，发送过来的是未变化前的状态，也就说客户端的
        //本意就是要变成server端现在的状态。
        //在处理 KEYEVENTF_EXTENDEDKEY 时，将evt.ki.wVk赋值了，可以用这个条件，也可以用 scancode==0x45
        if (evt.ki.wVk == VK_NUMLOCK) {
            printf("numlock status : %d, current status :%d", num_lock_status, curr_num_lock_status);
            if (curr_num_lock_status != num_lock_status) {
                printf("NumLock, NO need to send event.");
                return;
            }
        } else {
            //如果按的小键盘其他的按键，当down事件触发的时候，发现num lock的状态不一致，
            //那么需要在server端模拟一次num lock的按下，抬起动作，且要在发送此次事件之前
            if (event.down() && num_lock_status != curr_num_lock_status && event.status_check() == KeyEvent::kCheckNumLock) {
                MockKeyPressedByScanCode(0x45);
                //curr_num_lock_status = GetKeyState(VK_NUMLOCK);
                //LOGI("After send numlock down and up, current status : %d, client status : %d", curr_num_lock_status, num_lock_status);
            }
        }

        short caps_lock_status = event.caps_lock_status();
        std::cout << "event caps_lock_status = " << caps_lock_status << std::endl;
        short curr_caps_lock_status = GetKeyState(VK_CAPITAL);
        std::cout << "curr_caps_lock_status = " << curr_caps_lock_status << std::endl;
        if (scancode == 0x3a) {
            if (curr_caps_lock_status == caps_lock_status) {
                printf("CapsLock, No need to send event.\n");
                return;
            }
        } else {
            if (event.down() && caps_lock_status != curr_caps_lock_status && event.status_check() == KeyEvent::kCheckCapsLock) {
                MockKeyPressedByScanCode(0x3a);
                //curr_caps_lock_status = GetKeyState(VK_CAPITAL);
                //LOGI("After send capslock down and up, current status : %d, client status : %d", curr_caps_lock_status, caps_lock_status);
            }
        }
        WinSendEvent(&evt);
    }

    void WinEventReplayer::MockKeyPressedByScanCode(uint16_t scancode) {
        INPUT down = GenerateScanCodeInput(scancode, true, true);
        WinSendEvent(&down);
        INPUT up = GenerateScanCodeInput(scancode, false, true);
        WinSendEvent(&up);
    }

    void WinEventReplayer::HandleMouseEvent(const tc::MouseEvent& event) {
        float x_ratio = event.x_ratio();
        float y_ratio = event.y_ratio();
        // to do , 屏幕索引，后面要兼容多屏幕，目前暂用主屏
        int monitor_index = event.monitor_index();
        int button = event.button();
        int data = event.data();
        // to do 是否是全局，还是进程内部响应，暂定 截屏方式时 使用全局响应事件， 后续如果使用 WGC采集窗口的话，需要另外考虑
        if(Settings::Instance()->capture_.capture_video_type_ == Capture::kCaptureScreen) {
            PlayGlobalMouseEvent(monitor_index, x_ratio, y_ratio, button, data);
        }
    }

    void WinEventReplayer::PlayGlobalMouseEvent(int monitor_index, float x_ratio, float y_ratio, int buttons, int data) {
        if (monitors_.empty()) {
            LOGE("Don't have capturing monitor info.");
            return;
        }
        auto func_find_monitor = [&]() -> CaptureMonitorInfo {
            for (auto& mon : monitors_) {
                if (mon.index_ == monitor_index) {
                    return mon;
                }
            }
            return CaptureMonitorInfo{};
        };

        auto target_monitor = func_find_monitor();
        if (!target_monitor.Valid()) {
            LOGE("Invalid monitor for index: {}", monitor_index);
            return;
        }
//        LOGI("monitor idx: {}, left: {}, bottom: {}, v-left: {}, v-bottom: {}",
//             monitor_index, target_monitor.left_, target_monitor.bottom_,
//             target_monitor.virtual_left_, target_monitor.virtual_bottom_);

        int x = (int)(x_ratio * target_monitor.virtual_width_ + target_monitor.virtual_left_);
        int y = (int)(y_ratio * target_monitor.virtual_height_ + target_monitor.virtual_top_);
        INPUT evt;
        evt.type = INPUT_MOUSE;
        evt.mi.dx = x;
        evt.mi.dy = y;
        // to do ，这里暂定使用 绝对模式，虚拟桌面映射
        int target_buttons = 0;
        if (buttons & ButtonFlag::kMouseMove) {
            target_buttons |= MOUSEEVENTF_MOVE;
            LOGI("Move: {}x{}", x, y);
        }
        if (buttons & ButtonFlag::kLeftMouseButtonDown) {
            target_buttons |= MOUSEEVENTF_LEFTDOWN;
            LOGI("LeftDown: {}x{}", x, y);
        }
        if (buttons & ButtonFlag::kRightMouseButtonDown) {
            target_buttons |= MOUSEEVENTF_RIGHTDOWN;
        }
        if (buttons & ButtonFlag::kMiddleMouseButtonDown) {
            target_buttons |= MOUSEEVENTF_MIDDLEDOWN;
        }
        if (buttons & ButtonFlag::kLeftMouseButtonUp) {
            target_buttons |= MOUSEEVENTF_LEFTUP;
            LOGI("LeftUp: {}x{}", x, y);
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

    void WinEventReplayer::UpdateCaptureMonitorInfo(const CaptureMonitorInfoMessage& msg) {
        monitors_ = msg.monitors_;
    }
}
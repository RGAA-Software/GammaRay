//
// Created by Administrator on 2024/2/12.
//

#ifndef TC_APPLICATION_CONTROL_EVENT_REPLAYER_WIN_H
#define TC_APPLICATION_CONTROL_EVENT_REPLAYER_WIN_H
#include <memory>
#include "tc_message.pb.h"

namespace tc{
    class ControlEventReplayerWin {
    public:
        void HandleMessage(const std::shared_ptr<Message>& message);
        void HandleKeyEvent(const tc::KeyEvent& event);
        void HandleMouseEvent(const tc::MouseEvent& event);

        void PlayGlobalMouseEvent(int monitor_index, float x_ration, float y_ration, int buttonmask, int data);

        bool CheckKeyAllowDown(uint32_t vk,bool down);
        void ResetKey();

        void DoScanCodeEvent(uint16_t scancode, bool extend, const tc::KeyEvent& event);
        void MockKeyPressedByScanCode(uint16_t scancode);

        bool current_key_status_map[256] = {false, };




        bool control_pressed_ = false;
        bool menu_pressed_ = false;
        bool delete_pressed_ = false;
        bool shift_pressed_ = false;
        bool win_pressed_ = false;
    };
}


#endif //TC_APPLICATION_CONTROL_EVENT_REPLAYER_WIN_H

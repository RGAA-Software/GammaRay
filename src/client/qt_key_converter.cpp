//
// Created by RGAA on 2023/8/13.
//

#include "qt_key_converter.h"
#include "tc_common_new/log.h"

namespace tc
{
    QtKeyConverter::QtKeyConverter() = default;

    std::map<int, bool> QtKeyConverter::GetSysKeyStatus() {
        std::map<int, bool> status;
#if WIN32
        auto func_is_pressed = [](int vk) -> bool {
            return GetKeyState(vk) < 0;
        };
        status.insert(std::make_pair(VK_SHIFT, func_is_pressed(VK_SHIFT)));
        status.insert(std::make_pair(VK_LSHIFT, func_is_pressed(VK_LSHIFT)));
        status.insert(std::make_pair(VK_RSHIFT, func_is_pressed(VK_RSHIFT)));
        status.insert(std::make_pair(VK_SHIFT, func_is_pressed(VK_MBUTTON)));
        status.insert(std::make_pair(VK_CAPITAL, func_is_pressed(VK_CAPITAL)));
        status.insert(std::make_pair(VK_CONTROL, func_is_pressed(VK_CONTROL)));
        status.insert(std::make_pair(VK_MENU, func_is_pressed(VK_MENU)));
        status.insert(std::make_pair(VK_LMENU, func_is_pressed(VK_LMENU)));
        status.insert(std::make_pair(VK_RMENU, func_is_pressed(VK_RMENU)));
        status.insert(std::make_pair(VK_LBUTTON, func_is_pressed(VK_LBUTTON)));
        status.insert(std::make_pair(VK_RBUTTON, func_is_pressed(VK_RBUTTON)));
        status.insert(std::make_pair(VK_MBUTTON, func_is_pressed(VK_MBUTTON)));
#endif
        return status;
    }
}

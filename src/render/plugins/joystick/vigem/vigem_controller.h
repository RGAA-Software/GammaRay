//
// Created by RGAA on 2024/3/23.
//

#ifndef TC_APPLICATION_VIGEM_CONTROLLER_H
#define TC_APPLICATION_VIGEM_CONTROLLER_H

#include <memory>
#include <string>
#include <functional>
#include <Windows.h>
#include "sdk/ViGEm/Client.h"
#include "vigem_defs.h"

namespace tc
{

    enum class JoystickType {
        kJsX360,
        kJsDs4
    };

    class VigemController {
    public:
        VigemController(const JoystickType js_type, const std::string& stream_id);
        bool Connect();
        bool AllocController();
        void SendGamepadState(int index, const XInputGamepadState& state);
        void Exit();

        void MockPressB();

    private:
        JoystickType js_type_;
        std::string stream_id_;
        PVIGEM_CLIENT client_ = nullptr;
        PVIGEM_TARGET target_ = nullptr;
        bool target_connected_ = false;
    };

}

#endif //TC_APPLICATION_VIGEM_CONTROLLER_H

//
// Created by RGAA on 2024/3/23.
//

#ifndef TC_APPLICATION_VIGEM_CONTROLLER_H
#define TC_APPLICATION_VIGEM_CONTROLLER_H

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <Windows.h>
#include "sdk/ViGEm/Client.h"
#include "vigem_defs.h"

namespace tc
{

    enum class JoystickType {
        kJsX360,
        kJsDs4
    };

    //
    class VigemJoystick {
    public:
        std::string stream_id_;
        PVIGEM_TARGET target_ =  nullptr;
    };

    class VigemController {
    public:
        explicit VigemController(const JoystickType& js_type);
        bool Connect();
        bool IsConnected();
        bool AllocController(const std::string& stream_id);
        bool RemoveController(const std::string& stream_id);
        void SendGamepadState(const std::string& stream_id, const XInputGamepadState& state);
        void Exit();

        void MockPressB();

    private:
        JoystickType js_type_;
        PVIGEM_CLIENT client_ = nullptr;
        std::unordered_map<std::string, std::shared_ptr<VigemJoystick>> targets_;
    };

}

#endif //TC_APPLICATION_VIGEM_CONTROLLER_H

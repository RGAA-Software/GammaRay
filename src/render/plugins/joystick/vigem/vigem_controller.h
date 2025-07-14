//
// Created by RGAA on 2024/3/23.
//

#ifndef TC_APPLICATION_VIGEM_CONTROLLER_H
#define TC_APPLICATION_VIGEM_CONTROLLER_H

#include <memory>
#include <functional>

#include <Windows.h>
#include "sdk/ViGEm/Client.h"
#include "vigem_defs.h"

namespace tc
{

    class VigemController {
    public:

        static std::shared_ptr<VigemController> Make();

        bool Connect();
        bool AllocController();
        void SendGamepadState(int index, const XInputGamepadState& state);
        void Exit();

        void MockPressB();

    private:
        PVIGEM_CLIENT client_;
        PVIGEM_TARGET target_;
        bool target_connected_;
    };

}

#endif //TC_APPLICATION_VIGEM_CONTROLLER_H

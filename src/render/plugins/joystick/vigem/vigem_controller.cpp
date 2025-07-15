//
// Created by RGAA on 2024/3/23.
//

#include "vigem_controller.h"
#include "tc_common_new/log.h"

#include <Xinput.h>

namespace tc
{

    VigemController::VigemController(const JoystickType js_type, const std::string& stream_id) {
        js_type_ = js_type;
        stream_id_ = stream_id;
        LOGI("Joystick type: {}, stream id: {}", (int)js_type_, stream_id_);
    }

    bool VigemController::Connect() {
        client_ = vigem_alloc();
        if (!client_) {
            LOGE("Alloc vigem failed!!!");
            return false;
        }
        const auto ret = vigem_connect(client_);
        if (!VIGEM_SUCCESS(ret)) {
            LOGE("Connect failed !!!!");
            return false;
        }

        return true;
    }

    bool VigemController::AllocController() {
        if (js_type_ == JoystickType::kJsX360) {
            target_ = vigem_target_x360_alloc();
        }
        else {
            target_ = vigem_target_ds4_alloc();
        }
        if (!target_) {
            LOGE("Alloc joystick failed.");
            return false;
        }

        auto err = vigem_target_add(client_, target_);
        if (!VIGEM_SUCCESS(err)) {
            LOGE("vigem_target_add joystick failed: 0x{:x}", (int32_t)err);
            return false;
        }

        err = vigem_target_x360_register_notification(client_, target_, [](PVIGEM_CLIENT Client,
                                                                     PVIGEM_TARGET Target,
                                                                     UCHAR LargeMotor,
                                                                     UCHAR SmallMotor,
                                                                     UCHAR LedNumber,
                                                                     LPVOID UserData) {
//            const auto pad = static_cast<EmulationTarget*>(UserData);
//
//            XINPUT_VIBRATION vibration;
//            vibration.wLeftMotorSpeed = LargeMotor * 257;
//            vibration.wRightMotorSpeed = SmallMotor * 257;
//
//            g_pXInputSetState(pad->userIndex, &vibration);
        }, this);

        if (!VIGEM_SUCCESS(err)) {
            LOGE("vigem_target_x360_register_notification x360 failed: 0x{:x}", (int32_t)err);
            return false;
        }

        target_connected_ = vigem_target_is_attached(target_);
        LOGI("target connected: {}", target_connected_);
        return target_connected_;
    }

    void VigemController::SendGamepadState(int index, const XInputGamepadState &state) {
        XInputGamepadState* gs = const_cast<XInputGamepadState*>(&state);
        vigem_target_x360_update(client_, target_, *reinterpret_cast<XUSB_REPORT*>(gs));
    }

    void VigemController::Exit() {
        if (client_ && target_) {
            vigem_target_remove(client_, target_);
        }
        if (target_) {
            vigem_target_free(target_);
        }
        if (client_) {
            vigem_disconnect(client_);
            vigem_free(client_);
        }
    }

    void VigemController::MockPressB() {
        //xbox
        // The XINPUT_GAMEPAD structure is identical to the XUSB_REPORT structure
        // so we can simply take it "as-is" and cast it.
        XINPUT_GAMEPAD pad{};
        pad.wButtons |= XINPUT_GAMEPAD_X;
        pad.wButtons |= XINPUT_GAMEPAD_B;
        vigem_target_x360_update(client_, target_, *reinterpret_cast<XUSB_REPORT*>(&pad));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        pad.wButtons &= ~XINPUT_GAMEPAD_X;
        pad.wButtons &= ~XINPUT_GAMEPAD_B;
        vigem_target_x360_update(client_, target_, *reinterpret_cast<XUSB_REPORT*>(&pad));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

}

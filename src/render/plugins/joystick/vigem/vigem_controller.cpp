//
// Created by RGAA on 2024/3/23.
//

#include "vigem_controller.h"
#include "tc_common_new/log.h"

#include <Xinput.h>

namespace tc
{

    VigemController::VigemController(const JoystickType &js_type) {
        js_type_ = js_type;
        LOGI("Joystick type: {}", (int) js_type_);
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
            vigem_free(client_);
            client_ = nullptr;
            return false;
        }

        return true;
    }

    bool VigemController::IsConnected() {
        return client_ != nullptr;
    }

    bool VigemController::AllocController(const std::string &stream_id) {
        PVIGEM_TARGET target = nullptr;
        if (js_type_ == JoystickType::kJsX360) {
            target = vigem_target_x360_alloc();
        } else {
            target = vigem_target_ds4_alloc();
        }
        if (!target) {
            LOGE("Alloc joystick failed for stream : {}", stream_id);
            return false;
        }

        auto err = vigem_target_add(client_, target);
        if (!VIGEM_SUCCESS(err)) {
            LOGE("vigem_target_add joystick failed: 0x{:x}", (int32_t) err);
            return false;
        }

        err = vigem_target_x360_register_notification(client_, target, [](PVIGEM_CLIENT Client,
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
            LOGE("vigem_target_x360_register_notification x360 failed: 0x{:x}", (int32_t) err);
            return false;
        }

        auto target_connected = vigem_target_is_attached(target);
        if (target_connected) {
            targets_.insert({stream_id, std::make_shared<VigemJoystick>(VigemJoystick {
                .stream_id_ = stream_id,
                .target_ = target,
            })});
        }
        LOGI("target connected: {}", target_connected);
        return target_connected;
    }

    bool VigemController::RemoveController(const std::string& stream_id) {
        for (const auto &[sid, js]: targets_) {
            if (sid == stream_id) {
                if (client_ && js->target_) {
                    vigem_target_remove(client_, js->target_);
                }
                if (js->target_) {
                    vigem_target_free(js->target_);
                }
            }
        }
        targets_.erase(stream_id);
        return true;
    }

    void VigemController::SendGamepadState(const std::string &stream_id, const XInputGamepadState &state) {
        for (const auto &[sid, js]: targets_) {
            if (sid == stream_id) {
                XInputGamepadState *gs = const_cast<XInputGamepadState *>(&state);
                vigem_target_x360_update(client_, js->target_, *reinterpret_cast<XUSB_REPORT *>(gs));
            }
        }
    }

    void VigemController::Exit() {
        for (const auto &[stream_id, js]: targets_) {
            if (client_ && js->target_) {
                vigem_target_remove(client_, js->target_);
            }
            if (js->target_) {
                vigem_target_free(js->target_);
            }
        }
        if (client_) {
            vigem_disconnect(client_);
            vigem_free(client_);
            client_ = nullptr;
        }
    }

    void VigemController::MockPressB() {
        //xbox
        // The XINPUT_GAMEPAD structure is identical to the XUSB_REPORT structure
        // so we can simply take it "as-is" and cast it.
        for (const auto &[stream_id, js]: targets_) {
            XINPUT_GAMEPAD pad{};
            pad.wButtons |= XINPUT_GAMEPAD_X;
            pad.wButtons |= XINPUT_GAMEPAD_B;
            vigem_target_x360_update(client_, js->target_, *reinterpret_cast<XUSB_REPORT *>(&pad));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            pad.wButtons &= ~XINPUT_GAMEPAD_X;
            pad.wButtons &= ~XINPUT_GAMEPAD_B;
            vigem_target_x360_update(client_, js->target_, *reinterpret_cast<XUSB_REPORT *>(&pad));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        }

    }
}

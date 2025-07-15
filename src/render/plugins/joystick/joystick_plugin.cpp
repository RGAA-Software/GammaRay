//
// Created RGAA on 15/11/2024.
//

#include "joystick_plugin.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "vigem/vigem_controller.h"
#include "render/plugins/plugin_ids.h"
#include "plugin_interface/gr_plugin_events.h"
#include "plugin_interface/gr_plugin_context.h"

void* GetInstance() {
    static tc::JoystickPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    std::string JoystickPlugin::GetPluginId() {
        return kJoystickPluginId;
    }

    std::string JoystickPlugin::GetPluginName() {
        return "JoyStick";
    }

    std::string JoystickPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t JoystickPlugin::GetVersionCode() {
        return 110;
    }

    std::string JoystickPlugin::GetPluginDescription() {
        return plugin_desc_;
    }

    void JoystickPlugin::On1Second() {
        GrPluginInterface::On1Second();

    }
    
    bool JoystickPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrPluginInterface::OnCreate(param);

        if (!IsPluginEnabled()) {
            return true;
        }
        root_widget_->hide();

        return true;
    }

    void JoystickPlugin::OnMessage(std::shared_ptr<Message> msg) {
        GrPluginInterface::OnMessage(msg);
        auto stream_id = msg->stream_id();
        if (msg->type() == tc::MessageType::kHello) {
            auto sub = msg->hello();
            if (sub.enable_controller()) {
                plugin_context_->PostWorkTask([=, this]() {
                    InitJoystick(stream_id);
                });
            }
        }
        else if (msg->type() == tc::MessageType::kGamepadState) {
            // replay gamepad state
            plugin_context_->PostWorkTask([=, this]() {
                ReplayJoystickEvent(stream_id, msg);
            });
        }
    }

    void JoystickPlugin::OnClientDisconnected(const std::string &visitor_device_id, const std::string &stream_id) {
        if (auto controller = RemoveController(stream_id); controller) {
            LOGW("will release joystick controller for stream: {}, device id: {}", stream_id, visitor_device_id);
            controller->Exit();
        }
    }

    std::shared_ptr<VigemController> JoystickPlugin::FindController(const std::string& stream_id) {
        if (controllers_.contains(stream_id)) {
            return controllers_[stream_id];
        }
        return nullptr;
    }

    std::shared_ptr<VigemController> JoystickPlugin::RemoveController(const std::string& stream_id) {
        if (auto it = controllers_.find(stream_id); it != controllers_.end()) {
            auto c = std::move(it->second);
            controllers_.erase(it);
            return c;
        }
        return nullptr;
    }

    void JoystickPlugin::InitJoystick(const std::string& stream_id) {
        // remove old
        if (auto controller = RemoveController(stream_id); controller) {
            controller->Exit();
        }

        // create controller
        auto controller = std::make_shared<VigemController>(JoystickType::kJsX360, stream_id);
        if (!controller->Connect()) {
            return;
        }
        if (controller->AllocController()) {
            controllers_[stream_id] = controller;
        }
    }

    void JoystickPlugin::ReplayJoystickEvent(const std::string& stream_id, std::shared_ptr<Message> msg) {
        auto controller = FindController(stream_id);
        if (!controller) {
            LOGE("Can't find joystick controller for stream: {}", stream_id);
            return;
        }
        const auto& gamepad_state = msg->gamepad_state();
        // convert to XINPUT_STATE
        // LOGI("----Gamepad state----");
        // LOGI("button: {:x}, left trigger: {}, right trigger: {}", gamepad_state.buttons(), gamepad_state.left_trigger(), gamepad_state.right_trigger());
        // LOGI("Left thumb: {},{}", gamepad_state.thumb_lx(), gamepad_state.thumb_ly());
        // LOGI("Right thumb: {},{}", gamepad_state.thumb_rx(), gamepad_state.thumb_ry());

        XInputGamepadState state;
        state.wButtons = gamepad_state.buttons();
        state.bLeftTrigger = gamepad_state.left_trigger();
        state.bRightTrigger = gamepad_state.right_trigger();
        state.sThumbLX = gamepad_state.thumb_lx();
        state.sThumbLY = gamepad_state.thumb_ly();
        state.sThumbRX = gamepad_state.thumb_rx();
        state.sThumbRY = gamepad_state.thumb_ry();
        controller->SendGamepadState(0, state);
    }

}

//
// Created RGAA on 15/11/2024.
//

#include "joystick_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "render/plugins/plugin_ids.h"
#include "tc_message.pb.h"

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
                // create controller
            }
        }
        else if (msg->type() == tc::MessageType::kGamepadState) {
            // replay gamepad state
        }
    }

    void JoystickPlugin::OnClientDisconnected(const std::string &visitor_device_id, const std::string &stream_id) {

    }

}

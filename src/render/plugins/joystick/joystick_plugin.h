//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_JOYSTICK_PLUGIN_H
#define GAMMARAY_JOYSTICK_PLUGIN_H

#include <map>
#include "plugin_interface/gr_plugin_interface.h"

namespace tc
{

    class VigemController;

    class JoystickPlugin : public GrPluginInterface {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        std::string GetPluginDescription() override;
        bool OnCreate(const tc::GrPluginParam& param) override;
        void On1Second() override;
        void OnMessage(std::shared_ptr<Message> msg) override;
        void OnClientDisconnected(const std::string &visitor_device_id, const std::string &stream_id) override;

    private:
        void PrepareConnection();
        void ReplayJoystickEvent(const std::string& stream_id, std::shared_ptr<Message> msg);

    private:
        std::shared_ptr<VigemController> controller_;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

#endif

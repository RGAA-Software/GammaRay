//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_DDA_CAPTURE_PLUGIN_H
#define GAMMARAY_DDA_CAPTURE_PLUGIN_H

#include "plugin_interface/gr_monitor_capture_plugin.h"

namespace tc
{

    class DesktopCapture;

    class DDACapturePlugin : public GrMonitorCapturePlugin {
    public:
        DDACapturePlugin();
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        bool IsWorking() override;
        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        bool StartCapturing(const std::string& target) override;
        void StopCapturing() override;

    private:
        std::shared_ptr<tc::DesktopCapture> capture_ = nullptr;
        bool init_success_ = false;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();


#endif //GAMMARAY_UDP_PLUGIN_H

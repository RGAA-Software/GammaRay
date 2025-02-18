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
        bool StartCapturing() override;
        void StopCapturing() override;
        std::vector<CaptureMonitorInfo> GetCaptureMonitorInfo() override;
        int GetCapturingMonitorIndex() override;
        std::string GetCapturingMonitorName() override;
        void SetCaptureMonitor(int index, const std::string& name) override;
        void SetCaptureFps(int fps) override;
        void OnNewClientIn() override;

    private:
        void InitCaptures();

    private:
        // @Deprecated
        std::shared_ptr<tc::DesktopCapture> capture_ = nullptr;
        std::map<std::string, std::shared_ptr<DesktopCapture>> captures_;
        bool init_success_ = false;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();


#endif //GAMMARAY_UDP_PLUGIN_H

//
// Created by RGAA on 22/11/2024.
//

#include "gr_monitor_capture_plugin.h"

namespace tc
{

    GrMonitorCapturePlugin::GrMonitorCapturePlugin() {

    }

    bool GrMonitorCapturePlugin::OnCreate(const tc::GrPluginParam &param) {
        GrPluginInterface::OnCreate(param);
        return true;
    }

    bool GrMonitorCapturePlugin::OnDestroy() {
        GrPluginInterface::OnDestroy();
        return true;
    }

    bool GrMonitorCapturePlugin::StartCapturing() {
        return true;
    }

    void GrMonitorCapturePlugin::StopCapturing() {

    }

    bool GrMonitorCapturePlugin::IsMonitorTarget() {
        return is_monitor_target_;
    }

    std::vector<CaptureMonitorInfo> GrMonitorCapturePlugin::GetCaptureMonitorInfo() {
        return {};
    }

    void GrMonitorCapturePlugin::SetCaptureMonitor(const std::string& name) {

    }

    void GrMonitorCapturePlugin::SetCaptureFps(int fps) {
        capture_fps_ = fps;
    }

}
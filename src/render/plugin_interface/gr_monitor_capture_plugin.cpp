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
        return false;
    }

    void GrMonitorCapturePlugin::StopCapturing() {

    }

    std::vector<CaptureMonitorInfo> GrMonitorCapturePlugin::GetCaptureMonitorInfo() {
        return {};
    }

    void GrMonitorCapturePlugin::SetCaptureMonitor(const std::string& name) {

    }

    std::string GrMonitorCapturePlugin::GetCapturingMonitor() {
        return capturing_monitor_name_;
    }

    void GrMonitorCapturePlugin::SetCaptureFps(int fps) {
        capture_fps_ = fps;
    }

    void GrMonitorCapturePlugin::On16MilliSecond() {

    }

    void GrMonitorCapturePlugin::On33MilliSecond() {

    }

    void GrMonitorCapturePlugin::SetCaptureErrorCallback(const tc::CaptureErrorCallback& cbk) {
        capture_err_callback_ = cbk;
    }

}
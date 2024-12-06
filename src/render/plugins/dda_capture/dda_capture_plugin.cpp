//
// Created RGAA on 15/11/2024.
//

#include "dda_capture_plugin.h"
#include "render/plugins/plugin_ids.h"
#include "dda_capture.h"
#include "tc_common_new/log.h"

static void* GetInstance() {
    static tc::DDACapturePlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    DDACapturePlugin::DDACapturePlugin() : GrMonitorCapturePlugin() {

    }

    std::string DDACapturePlugin::GetPluginId() {
        return kDdaCapturePluginId;
    }

    std::string DDACapturePlugin::GetPluginName() {
        return "DDA Capture Plugin";
    }

    std::string DDACapturePlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t DDACapturePlugin::GetVersionCode() {
        return 110;
    }

    bool DDACapturePlugin::OnCreate(const tc::GrPluginParam& param) {
        GrMonitorCapturePlugin::OnCreate(param);
        return true;
    }

    bool DDACapturePlugin::OnDestroy() {
        return false;
    }

    bool DDACapturePlugin::IsWorking() {
        return capture_ && init_success_;
    }

    bool DDACapturePlugin::StartCapturing(const std::string& target) {
        capture_ =  std::make_shared<DDACapture>(this, target);
        if (!capture_->Init()) {
            LOGE("dda capture init failed for target: {}", target);
            //msg_notifier->SendAppMessage(CaptureInitFailedMessage {});
            return false;
        }
        init_success_ = true;
        return capture_->StartCapture();
    }

    void DDACapturePlugin::StopCapturing() {
        init_success_ = false;
    }

    std::vector<CaptureMonitorInfo> DDACapturePlugin::GetCaptureMonitorInfo() {
        if (!IsWorking()) {
            return GrMonitorCapturePlugin::GetCaptureMonitorInfo();
        }
        return capture_->GetCaptureMonitorInfo();
    }

    int DDACapturePlugin::GetCapturingMonitorIndex() {
        if (!IsWorking()) {
            return GrMonitorCapturePlugin::GetCapturingMonitorIndex();
        }
        return capture_->GetCapturingMonitorIndex();
    }

    std::string DDACapturePlugin::GetCapturingMonitorName() {
        if (!IsWorking()) {
            return GrMonitorCapturePlugin::GetCapturingMonitorName();
        }
        return capture_->GetCapturingMonitorName();
    }

    void DDACapturePlugin::SetCaptureMonitor(int index, const std::string& name) {
        if (IsWorking()) {
            capture_->SetCaptureMonitor(index, name);
        }
    }

    void DDACapturePlugin::SetCaptureFps(int fps) {
        if (IsWorking()) {
            capture_->SetCaptureFps(fps);
        }
    }

}

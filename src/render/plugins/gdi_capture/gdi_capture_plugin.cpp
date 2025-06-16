
#include "gdi_capture_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "tc_common_new/math_helper.h"
#include "render/plugins/plugin_ids.h"
#include "gdi_capture.h"

void* GetInstance() {
    static tc::GdiCapturePlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    GdiCapturePlugin::GdiCapturePlugin() : GrMonitorCapturePlugin() {

    }

    std::string GdiCapturePlugin::GetPluginId() {
        return kGdiCapturePluginId;
    }

    std::string GdiCapturePlugin::GetPluginName() {
        return "GDI";
    }

    std::string GdiCapturePlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t GdiCapturePlugin::GetVersionCode() {
        return 110;
    }

    std::string GdiCapturePlugin::GetPluginDescription() {
        return "GDI desktop capture";
    }

    void GdiCapturePlugin::On1Second() {
        GrPluginInterface::On1Second();
    }
    
    bool GdiCapturePlugin::OnCreate(const tc::GrPluginParam &param) {
        GrMonitorCapturePlugin::OnCreate(param);
        LOGI("GdiCapturePlugin OnCreate");
        CreateCapture();
        return true;
    }

    std::optional<int> GdiCapturePlugin::GetMonIndexByName(const std::string& name) {
        // gdi 采集整个虚拟桌面,所以返回0即可
        return { 0 };
    }

    void GdiCapturePlugin::DispatchAppEvent(const std::shared_ptr<AppBaseEvent>& event) {
        GrPluginInterface::DispatchAppEvent(event);
        //LOGI("GdiCapturePlugin DispatchAppEvent type: {}", static_cast<int>(event->type_));
        if (!event) {
            return;
        }
        switch (event->type_)
        {
        case AppBaseEvent::EType::kDisplayDeviceChange: {
            LOGI("GdiCapturePlugin DispatchAppEvent is kDisplayDeviceChange");
            HandleDisplayDeviceChangeEvent();
            break;
        }
        default:
            break;
        }
    }

    std::map<std::string, WorkingCaptureInfoPtr> GdiCapturePlugin::GetWorkingCapturesInfo() {
        std::map<std::string, WorkingCaptureInfoPtr> result;
        if(!gdi_capture_){
            return result;
        }
        if (gdi_capture_->IsPausing()) {
            return result;
        }

        const auto& my_monitor = gdi_capture_->GetMyMonitorInfo();
        result.insert({ kVirtualDesktopNameSign, std::make_shared<WorkingCaptureInfo>(WorkingCaptureInfo {
            .target_name_ = kVirtualDesktopNameSign,
            .fps_ = gdi_capture_->GetCapturingFps(),
            .capture_type_ = kCaptureTypeGDI,
            .capture_frame_width_ = my_monitor.VirtualDesktopWidth(),
            .capture_frame_height_ = my_monitor.VirtualDesktopHeight(),
            .capture_gaps_ = gdi_capture_->GetCaptureGaps(),
        }) });

        return result;
    }

    std::string GdiCapturePlugin::GetCapturingMonitorName() {
        return kVirtualDesktopNameSign;
    }
    
    bool GdiCapturePlugin::StartCapturing() {
        GrMonitorCapturePlugin::StartCapturing();
        StopCapturing();
        capturing_monitor_name_ = kVirtualDesktopNameSign;
        gdi_capture_->SetCaptureFps(capture_fps_);
        gdi_capture_->StartCapture();
        NotifyCaptureMonitorInfo();
        return true;
    }

    void GdiCapturePlugin::CreateCapture() {
        CaptureMonitorInfo cap_mon_info;
        cap_mon_info.virtual_desktop_left_ = GetSystemMetrics(SM_XVIRTUALSCREEN);
        cap_mon_info.virtual_desktop_top_ = GetSystemMetrics(SM_YVIRTUALSCREEN);
        cap_mon_info.virtual_desktop_width_ = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        cap_mon_info.virtual_desktop_height_ = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        cap_mon_info.virtual_desktop_width_ = MathHelper::AlignTo4Bytes(cap_mon_info.virtual_desktop_width_); // 直接4字节对齐, 方便后面直接进行内存copy,不然还得进行 '行' 内存拷贝
        cap_mon_info.name_ = kVirtualDesktopNameSign;
        gdi_capture_ = GdiCapture::Make(this, cap_mon_info);
    }

    void GdiCapturePlugin::HandleDisplayDeviceChangeEvent() {
        RestartCapturing();
    }

    void GdiCapturePlugin::RestartCapturing() {
        LOGI("GdiCapturePlugin RestartCapturing");
        StopCapturing();
        gdi_capture_ = nullptr;
        CreateCapture();
        StartCapturing();
    }

    void GdiCapturePlugin::StopCapturing() {
        if (!gdi_capture_) {
            gdi_capture_->StopCapture();
        }
    }

    void GdiCapturePlugin::NotifyCaptureMonitorInfo() {
        auto event = std::make_shared<GrPluginCapturingMonitorInfoEvent>();
        this->CallbackEvent(event);
    }

}

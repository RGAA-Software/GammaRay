
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
        return "Gdi Capture";
    }

    std::string GdiCapturePlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t GdiCapturePlugin::GetVersionCode() {
        return 110;
    }

    void GdiCapturePlugin::On1Second() {
        GrPluginInterface::On1Second();

    }
    
    bool GdiCapturePlugin::OnCreate(const tc::GrPluginParam &param) {
        GrMonitorCapturePlugin::OnCreate(param);
        LOGI("GdiCapturePlugin OnCreate");
      /*  InitVideoCaptures();
        InitCursorCapture();*/
       // LOGI("DDA Capture audio device: {}", capture_audio_device_id_);


        CreateCapture();

        return true;
    }


    std::optional<int> GdiCapturePlugin::GetMonIndexByName(const std::string& name) {
       /* int mon_index = 0;
        for (auto monitor : sorted_monitors_) {
            if (name == monitor.name_) {
                return { mon_index };
            }
            ++mon_index;
        }*/
        return { std::nullopt };
    }

    void GdiCapturePlugin::DispatchAppEvent(const std::shared_ptr<AppBaseEvent>& event) {
        LOGI("GdiCapturePlugin DispatchAppEvent type: {}", static_cast<int>(event->type_));
        if (!event) {
            return;
        }
        switch (event->type_)
        {
        case AppBaseEvent::EType::kDisplayDeviceChange: {
            LOGI("GdiCapturePlugin DispatchAppEvent is kDisplayDeviceChange");
            //HandleDisplayDeviceChangeEvent();
            break;
        }
        default:
            break;
        }
    }


    std::map<std::string, WorkingCaptureInfoPtr> GdiCapturePlugin::GetWorkingCapturesInfo() {
        std::map<std::string, WorkingCaptureInfoPtr> result;
        /*for (const auto& [name, capture] : captures_) {
            if (capture->IsPausing()) {
                continue;
            }
            const auto& my_monitor = capture->GetMyMonitorInfo();
            result.insert({ name, std::make_shared<WorkingCaptureInfo>(WorkingCaptureInfo {
                .target_name_ = name,
                .fps_ = capture->GetCapturingFps(),
                .capture_type_ = kCaptureTypeDXGI,
                .capture_frame_width_ = my_monitor.Width(),
                .capture_frame_height_ = my_monitor.Height(),
                .capture_gaps_ = capture->GetCaptureGaps(),
            }) });
        }*/
        return result;
    }


    std::string GdiCapturePlugin::GetCapturingMonitorName() {
        return kVirtualDesktopNameSign;
    }
    

    bool GdiCapturePlugin::StartCapturing() {
        
      /*  if (!gdi_capture_->Init()) {
            return false;
        }*/

        gdi_capture_->StartCapture();







        
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

}

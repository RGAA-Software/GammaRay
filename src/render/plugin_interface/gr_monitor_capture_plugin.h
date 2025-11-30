//
// Created by RGAA on 22/11/2024.
//

#ifndef GAMMARAY_GR_MONITOR_CAPTURE_PLUGIN_H
#define GAMMARAY_GR_MONITOR_CAPTURE_PLUGIN_H

#include "gr_plugin_interface.h"
#include <optional>
#include "tc_capture_new/monitor_util.h"
#include "gr_monitor_capture_error.h"

namespace tc
{
    const std::string kAllMonitorsNameSign = "all";
    const std::string kCaptureTypeDXGI = "DXGI";
    const std::string kCaptureTypeGDI = "GDI";
    const std::string kCaptureTypeHook = "INNER";

    class Image;
    class Data;

    // dynamic working capture information
    class WorkingCaptureInfo {
    public:
        // monitor name or hook capturing
        std::string target_name_;
        int32_t fps_ = 0;
        // DXGI / GDI / HOOK
        std::string capture_type_;
        int32_t capture_frame_width_ = 0;
        int32_t capture_frame_height_ = 0;
        // max 180
        std::vector<int32_t> capture_gaps_;
    };
    using WorkingCaptureInfoPtr = std::shared_ptr<WorkingCaptureInfo>;

    class GrMonitorCapturePlugin : public GrPluginInterface {
    public:
        GrMonitorCapturePlugin();

        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        void SetCaptureErrorCallback(const CaptureErrorCallback& cbk);
        
        // target: monitor
        virtual bool TryInitSpecificCapture() = 0;
        virtual bool StartCapturing() = 0;
        virtual void StopCapturing() = 0;

        virtual std::vector<CaptureMonitorInfo> GetCaptureMonitorInfo();
        virtual VirtualDesktopBoundRectangleInfo GetVirtualDesktopBoundRectangleInfo() { return VirtualDesktopBoundRectangleInfo{}; };
        virtual void SetCaptureMonitor(const std::string& name);
        // capturing monitor
        std::string GetCapturingMonitor();
        // capturing information for monitors/hook
        virtual std::map<std::string, WorkingCaptureInfoPtr> GetWorkingCapturesInfo() = 0;

        virtual void SetCaptureFps(int fps);
        virtual std::string GetCapturingMonitorName() = 0;
        //根据显示器名字获取排序位置
        virtual std::optional<int> GetMonIndexByName(const std::string& name) = 0;

        virtual void HandleDisplayDeviceChangeEvent() = 0;

        virtual void On16MilliSecond();
        virtual void On33MilliSecond();

        static bool IsValidRect(const RECT& rect) {
            return rect.right > rect.left && rect.bottom > rect.top;
        }

    protected:
        int capture_fps_ = 60;
        std::string capturing_monitor_name_;
        CaptureErrorCallback capture_err_callback_;
    };
}

#endif //GAMMARAY_GR_MONITOR_CAPTURE_PLUGIN_H

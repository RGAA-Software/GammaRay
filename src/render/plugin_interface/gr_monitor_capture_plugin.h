//
// Created by RGAA on 22/11/2024.
//

#ifndef GAMMARAY_GR_MONITOR_CAPTURE_PLUGIN_H
#define GAMMARAY_GR_MONITOR_CAPTURE_PLUGIN_H

#include "gr_plugin_interface.h"
#include <optional>
#include "hook_capture/win/desktop_capture/monitor_util.h"

namespace tc
{
    const std::string kAllMonitorsNameSign = "all";
    const std::string kVirtualDesktopNameSign = "Virtual_Desktop"; // 目前此常量用于 GDI采集时候,创建DC设备时候,是根据显示器设备名来创建DC设备, 还是直接GetDC,如果直接GetDC,那采集的就是整个虚拟桌面
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

        using CaptureInitFailedCallback = std::function<void()>;
        CaptureInitFailedCallback capture_init_failed_cbk_;
        void SetCaptureInitFailedCallback(CaptureInitFailedCallback&& cbk) {
            capture_init_failed_cbk_ = std::move(cbk);
        }
        
        // target: monitor
        virtual bool StartCapturing();
        virtual void StopCapturing();

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
    protected:
        int capture_fps_ = 60;
        std::string capturing_monitor_name_;
    };
}

#endif //GAMMARAY_GR_MONITOR_CAPTURE_PLUGIN_H

//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_DDA_CAPTURE_PLUGIN_H
#define GAMMARAY_DDA_CAPTURE_PLUGIN_H
#include <optional>
#include "plugin_interface/gr_monitor_capture_plugin.h"
#include "tc_common_new/concurrent_hashmap.h"

namespace tc
{

    class Thread;
    class CursorCapture;
    class PluginDesktopCapture;

    class DDACapturePlugin : public GrMonitorCapturePlugin {
    public:
        DDACapturePlugin();
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        std::string GetPluginDescription() override;
        bool IsWorking() override;
        bool OnCreate(const tc::GrPluginParam& param) override;
        bool OnDestroy() override;
        bool TryInitSpecificCapture() override;
        bool StartCapturing() override;
        void StopCapturing() override;
        std::vector<CaptureMonitorInfo> GetCaptureMonitorInfo() override;
        std::string GetCapturingMonitorName() override;
        void SetCaptureMonitor(const std::string& name) override;
        void SetCaptureFps(int fps) override;
        void OnNewClientConnected(const std::string& visitor_device_id, const std::string& stream_id, const std::string& conn_type) override;
        void On1Second() override;
        void On16MilliSecond() override;
        void On33MilliSecond() override;

        //根据显示器名字获取排序位置
        std::optional<int> GetMonIndexByName(const std::string& name) override;

        void DispatchAppEvent(const std::shared_ptr<AppBaseEvent>& event) override;

        VirtualDesktopBoundRectangleInfo GetVirtualDesktopBoundRectangleInfo() override;

        // Capturing target <==> information
        std::map<std::string, WorkingCaptureInfoPtr> GetWorkingCapturesInfo() override;

    private:
        bool InitVideoCaptures();
        void InitCursorCapture();
        std::vector<SupportedResolution> GetSupportedResolutions(const std::wstring& name);
        void CalculateVirtualDeskInfo();
        void NotifyCaptureMonitorInfo();

        void RestartCapturing();

        void HandleDisplayDeviceChangeEvent() override;

        bool ExistCaptureMonitor(const std::string& name);

    private:
        std::map<std::string, CaptureMonitorInfo> monitors_;
        tc::ConcurrentHashMap<std::string, std::shared_ptr<PluginDesktopCapture>> captures_;
        std::vector<CaptureMonitorInfo> sorted_monitors_;
        std::shared_ptr<CursorCapture> cursor_capture_ = nullptr;
        std::shared_ptr<Thread> cursor_capture_thread_ = nullptr;

        VirtualDesktopBoundRectangleInfo virtual_desktop_bound_rectangle_info_;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();


#endif //GAMMARAY_UDP_PLUGIN_H


#pragma once

#include "plugin_interface/gr_monitor_capture_plugin.h"
#include <map>
#include <string>
#include <memory>

namespace tc
{

    class PluginDesktopCapture;

    class GdiCapturePlugin : public GrMonitorCapturePlugin {
    public:
        GdiCapturePlugin();
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;

        bool OnCreate(const tc::GrPluginParam& param) override;
        void On1Second() override;
    
        void DispatchAppEvent(const std::shared_ptr<AppBaseEvent>& event) override;

        // Capturing target <==> information
        std::map<std::string, WorkingCaptureInfoPtr> GetWorkingCapturesInfo() override;

        std::string GetCapturingMonitorName() override;

        //根据显示器名字获取排序位置
        std::optional<int> GetMonIndexByName(const std::string& name) override;


        bool StartCapturing() override;

    private:
        void CreateCapture();

    private:
        std::map<std::string, QLabel*> previewers_;

        std::shared_ptr<PluginDesktopCapture> gdi_capture_ = nullptr;
    };


}

extern "C" __declspec(dllexport) void* GetInstance();





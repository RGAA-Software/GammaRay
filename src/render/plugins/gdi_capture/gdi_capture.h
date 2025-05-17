#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include "tc_common_new/monitors.h"
#include "tc_common_new/fps_stat.h"
#include "plugins/plugin_desktop_capture.h"

namespace tc
{
    class Data;
    class GdiCapturePlugin;

    class GdiCapture : public PluginDesktopCapture {
    public:
        static std::shared_ptr<GdiCapture> Make(GdiCapturePlugin* plugin, const CaptureMonitorInfo& my_monitor_info);
        explicit GdiCapture(GdiCapturePlugin* plugin, const CaptureMonitorInfo& my_monitor_info);
        virtual ~GdiCapture();
        bool Init() override;
        bool StartCapture() override;
        bool PauseCapture() override;
        void ResumeCapture() override;
        void StopCapture() override;
        void RefreshScreen() override;
        bool IsPrimaryMonitor() override;
        bool IsInitSuccess() override;
        int GetCapturingFps() override;

        using GdiInitSuccessCallback = std::function<void()>;
        GdiInitSuccessCallback gdi_init_success_callback_ = nullptr;
        void SetDDAInitSuccessCallback(GdiInitSuccessCallback&& cbk) {
            gdi_init_success_callback_ = std::move(cbk);
        }
    private:
        void Start();
        bool Exit();
        void Capture();
        //HRESULT CaptureNextFrame(int wait_time, CComPtr<ID3D11Texture2D>& out_tex);
        //void OnCaptureFrame(ID3D11Texture2D* texture, bool is_cached);

        bool CaptureNextFrame();
        void OnCaptureFrame();


        void SendTextureHandle(/*const HANDLE& shared_handle, uint32_t width, uint32_t height, DXGI_FORMAT format*/);
        int GetFrameIndex();
        std::vector<SupportedResolution> GetSupportedResolutions(const std::wstring& name);

    private:
        bool init_success_ = false;


        GdiCapturePlugin* plugin_ = nullptr;

        std::atomic<bool> stop_flag_ = false;
        std::thread capture_thread_;
        int64_t monitor_frame_index_ = 0;
        int used_cache_times_ = 0;
        std::shared_ptr<FpsStat> fps_stat_ = nullptr;
        int64_t last_captured_timestamp_ = 0;

        //std::shared_ptr<Data> data_ptr_ = nullptr;

        

        HBITMAP bit_map_ = nullptr;
        HDC screen_dc_ = nullptr;
        HDC memory_dc_ = nullptr;


    };
}
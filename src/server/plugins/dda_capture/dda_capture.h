#pragma once

#include <atlbase.h>
#include <wrl/client.h>
#include <string>
#include <DXGI.h>
#include <d3d11.h>
#include <DXGI1_2.h>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <map>

#include "desktop_capture.h"
#include "tc_common_new/monitors.h"

using namespace Microsoft::WRL;

namespace tc
{
    class DDACapturePlugin;
    class CursorCapture;

    class DDACapture : public DesktopCapture  {
    public:
        enum class CaptureResult {
            kSuccess,
            kFailed,
            kReInit,
            kTryAgain,
        };

        class DXGIOutputDuplication {
        public:
            CComPtr<IDXGIOutputDuplication> duplication_;
            DXGI_OUTPUT_DESC output_desc_{};
            CaptureMonitorInfo monitor_win_info_{};

            DXGIOutputDuplication() {
                memset(&output_desc_, 0, sizeof(DXGI_OUTPUT_DESC));
            }
        };

        class SharedD3d11Texture2D {
        public:
            HANDLE shared_handle_ = nullptr;
            ComPtr<ID3D11Texture2D> texture2d_ = nullptr;
        };

        explicit DDACapture(DDACapturePlugin* plugin, const std::string& monitor);
        virtual ~DDACapture();
        bool Init() override;
        bool StartCapture() override;
        void StopCapture() override;

    private:
        void Start();
        bool Exit();
        void Capture();
        CaptureResult CaptureNextFrame(int wait_time, CComPtr<ID3D11Texture2D>& out_tex, int monitor_index = 0);
        void OnCaptureFrame(ID3D11Texture2D *texture, uint8_t monitor_index);
        void SendTextureHandle(const HANDLE &shared_handle, MonitorIndex monitor_index, uint32_t width, uint32_t height, DXGI_FORMAT format);
        int GetFrameIndex(MonitorIndex monitor_index);
        bool IsTargetMonitor(int index);
        void CalculateVirtualDeskInfo();
        std::vector<SupportedResolution> GetSupportedResolutions(const std::wstring& name);

    private:
        std::atomic<bool> stop_flag_ = false;
        std::thread capture_thread_;
        uint8_t monitor_count_ = 0;
        int64_t adapter_uid_ = -1;
        std::map<MonitorIndex, int> monitor_frame_index_;
        std::map<MonitorIndex, CaptureMonitorInfo> monitors_;
        std::vector<MonitorWinInfo> win_monitors_;

        std::map<MonitorIndex, SharedD3d11Texture2D> last_list_texture_;
        std::map<MonitorIndex, DXGIOutputDuplication> dxgi_output_duplication_;
        std::map<MonitorIndex, CComPtr<ID3D11Texture2D>> cached_textures_;
        CComPtr<IDXGIFactory1> factory1_ = nullptr;
        CComPtr<IDXGIAdapter1> adapter1_ = nullptr;
        CComPtr<IDXGIOutput> dxgi_output_ = nullptr;
        CComPtr<ID3D11Device> d3d11_device_ = nullptr;
        CComPtr<ID3D11DeviceContext> d3d11_device_context_ = nullptr;
        CComPtr<ID3D11Texture2D> shared_texture_ = nullptr;
        std::shared_ptr<CursorCapture> cursor_capture_ = nullptr;

    };
}
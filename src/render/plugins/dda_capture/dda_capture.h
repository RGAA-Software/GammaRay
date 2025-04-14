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
#include <functional>

#include "desktop_capture.h"
#include "tc_common_new/monitors.h"

using namespace Microsoft::WRL;

namespace tc
{
    class DDACapturePlugin;

    class DDACapture : public DesktopCapture  {
    public:

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

        explicit DDACapture(DDACapturePlugin* plugin, const CaptureMonitorInfo& my_monitor_info);
        virtual ~DDACapture();
        bool Init() override;
        bool StartCapture() override;
        bool PauseCapture() override;
        void ResumeCapture() override;
        void StopCapture() override;
        void RefreshScreen() override;
        bool IsPrimaryMonitor() override;
        bool IsInitSuccess() override;

        using DDAInitSuccessCallback = std::function<void()>;
        DDAInitSuccessCallback dda_init_success_callback_ = nullptr;
        void SetDDAInitSuccessCallback(DDAInitSuccessCallback&& cbk){
            dda_init_success_callback_ = std::move(cbk);
        }
    private:
        void Start();
        bool Exit();
        void Capture();
        HRESULT CaptureNextFrame(int wait_time, CComPtr<ID3D11Texture2D>& out_tex);
        void OnCaptureFrame(ID3D11Texture2D *texture, bool is_cached);
        void SendTextureHandle(const HANDLE &shared_handle, uint32_t width, uint32_t height, DXGI_FORMAT format);
        int GetFrameIndex();
        std::vector<SupportedResolution> GetSupportedResolutions(const std::wstring& name);

    private:
        std::atomic<bool> stop_flag_ = false;
        std::thread capture_thread_;
        int64_t monitor_frame_index_ = 0;
        int used_cache_times_ = 0;

        SharedD3d11Texture2D last_list_texture_;
        DXGIOutputDuplication dxgi_output_duplication_;
        CComPtr<ID3D11Texture2D> cached_texture_ = nullptr;

        CComPtr<ID3D11Device> d3d11_device_ = nullptr;
        CComPtr<ID3D11DeviceContext> d3d11_device_context_ = nullptr;
        CComPtr<ID3D11Texture2D> shared_texture_ = nullptr;

    };
}
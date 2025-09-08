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
#include "tc_common_new/monitors.h"
#include "tc_common_new/fps_stat.h"
#include "tc_common_new/win32/d3d11_wrapper.h"
#include "plugins/plugin_desktop_capture.h"

using namespace Microsoft::WRL;

namespace tc
{
    class DDACapturePlugin;

    class DDACapture : public PluginDesktopCapture  {
    public:

        class DXGIOutputDuplication {
        public:
            ComPtr<IDXGIOutput1> output1_ = nullptr;
            ComPtr<IDXGIOutputDuplication> duplication_ = nullptr;
            DXGI_OUTPUT_DESC output_desc_{};
            CaptureMonitorInfo monitor_win_info_{};
            bool has_retry_ = false;

            DXGIOutputDuplication() {
                memset(&output_desc_, 0, sizeof(DXGI_OUTPUT_DESC));
            }

            void Exit() {
                has_retry_ = false;
                if (output1_) {
                    // output1_.Release();
                    // output1_ = nullptr;
                    output1_.Reset();
                }
                if (duplication_) {
                    duplication_->ReleaseFrame();
                    duplication_.Reset();
                    //duplication_.Release();
                    //duplication_ = nullptr;
                }
                memset(&output_desc_, 0, sizeof(DXGI_OUTPUT_DESC));
                monitor_win_info_ = CaptureMonitorInfo{};
            }
        };

        class SharedD3d11Texture2D {
        public:
            void Exit() {
                if (texture2d_) {
                    //texture2d_.Release();
                    //texture2d_ = nullptr;
                    texture2d_.Reset();
                }
                shared_handle_ = nullptr;
            }

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
        int GetCapturingFps() override;
        void TryWakeOs() override;
        void On16MilliSecond() override;
        void On33MilliSecond() override;

        using DDAInitCallback = std::function<void(bool)>;
        DDAInitCallback dda_init_callback_ = nullptr;
        void SetDDAInitCallback(DDAInitCallback&& cbk){
            dda_init_callback_ = std::move(cbk);
        }
        int32_t GetContinuousTimeoutTimes();

    private:
        void Start();
        bool Exit();
        void Capture();
        HRESULT CaptureNextFrame(int wait_time, ComPtr<ID3D11Texture2D>& out_tex);
        void OnCaptureFrame(const ComPtr<ID3D11Texture2D>& texture, bool is_cached);
        void SendTextureHandle(const HANDLE &shared_handle, uint32_t width, uint32_t height, DXGI_FORMAT format);
        int64_t GetFrameIndex();

    private:
        DDACapturePlugin* plugin_ = nullptr;

        std::atomic<bool> stop_flag_ = false;
        std::thread capture_thread_;
        int64_t monitor_frame_index_ = 0;
        int used_cache_times_ = 0;
        std::shared_ptr<FpsStat> fps_stat_ = nullptr;
        int64_t last_captured_timestamp_ = 0;

        std::shared_ptr<SharedD3d11Texture2D> last_list_texture_ = nullptr;
        DXGIOutputDuplication dxgi_output_duplication_;
        ComPtr<ID3D11Texture2D> cached_texture_ = nullptr;

        ComPtr<ID3D11Device> d3d11_device_ = nullptr;
        ComPtr<ID3D11DeviceContext> d3d11_device_context_ = nullptr;

        std::atomic<int32_t> continuous_timeout_times_ = 0;
    };
}
//
// Created RGAA on 15/11/2024.
//

#include <ShlObj_core.h>
#include "dda_capture_plugin.h"
#include "render/plugins/plugin_ids.h"
#include "dda_capture.h"
#include "tc_common_new/log.h"
#include "tc_common_new/thread.h"
#include "plugin_interface/gr_plugin_events.h"
#include "cursor_capture.h"

static void* GetInstance() {
    static tc::DDACapturePlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    DDACapturePlugin::DDACapturePlugin() : GrMonitorCapturePlugin() {

    }

    std::string DDACapturePlugin::GetPluginId() {
        return kDdaCapturePluginId;
    }

    std::string DDACapturePlugin::GetPluginName() {
        return "DDA Capture Plugin";
    }

    std::string DDACapturePlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t DDACapturePlugin::GetVersionCode() {
        return 110;
    }

    bool DDACapturePlugin::OnCreate(const tc::GrPluginParam& param) {
        GrMonitorCapturePlugin::OnCreate(param);
        InitVideoCaptures();
        InitCursorCapture();
        LOGI("DDA Capture audio device: {}", capture_audio_device_id_);
        return true;
    }

    void DDACapturePlugin::InitVideoCaptures() {
        HRESULT res = 0;
        int adapter_index = 0;
        CComPtr<IDXGIFactory1> factory1_ = nullptr;
        res = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **) &factory1_);
        if (res != S_OK) {
            LOGE("!!! CreateDXGIFactory1 failed, this plugin can't work !!!");
            return;
        }
        
        do {
            CComPtr<IDXGIAdapter1> adapter1 = nullptr;
            CComPtr<ID3D11Device> d3d11_device = nullptr;
            CComPtr<ID3D11DeviceContext> d3d11_device_context = nullptr;
            LOGI("Will query Adapter: {}", adapter_index);
            res = factory1_->EnumAdapters1(adapter_index, &adapter1);
            if (res != S_OK) {
                LOGE("EnumAdapters1 failed, index: {}", adapter_index);
                break;
            }
            D3D_FEATURE_LEVEL feature_level;
            DXGI_ADAPTER_DESC adapter_desc{};
            adapter1->GetDesc(&adapter_desc);
            auto adapter_uid = adapter_desc.AdapterLuid.LowPart;
            LOGI("Adapter Index:{} Name:{}, Uid:{}", adapter_index, StringExt::ToUTF8(adapter_desc.Description).c_str(), adapter_uid);
            res = D3D11CreateDevice(adapter1, D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                    nullptr, 0, D3D11_SDK_VERSION, &d3d11_device, &feature_level,
                                    &d3d11_device_context);
            if (res != S_OK || !d3d11_device) {
                LOGE("D3D11CreateDevice failed: {}", StringExt::GetErrorStr(res).c_str());
                break;
            }
            if (feature_level < D3D_FEATURE_LEVEL_11_0) {
                LOGE("D3D11CreateDevice returns an instance without DirectX 11 support, level : {}  Following initialization may fail",(int) feature_level);
                break;
            }
            CComPtr<IDXGIDevice> dxgi_device;
            res = d3d11_device.QueryInterface(&dxgi_device);
            if (res != S_OK || !dxgi_device) {
                LOGE("ID3D11Device is not an implementation of IDXGIDevice, this usually means the system does not support DirectX 11. Error:{}, code: {}",
                     StringExt::GetErrorStr(res), res);
                break;
            }

            int monitor_index = 0;
            do {
                CComPtr<IDXGIOutput> output;
                res = adapter1->EnumOutputs(monitor_index, &output);
                if (res == DXGI_ERROR_NOT_FOUND) {
                    LOGE("adapter1->EnumOutputs return DXGI_ERROR_NOT_FOUND,Please Check RDP connect.");
                    break;
                }
                if (res == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) {
                    LOGE("IDXGIAdapter::EnumOutputs returns NOT_CURRENTLY_AVAILABLE. This may happen when running in session 0");
                    break;
                }
                if (res != S_OK || !output) {
                    LOGE("IDXGIAdapter::EnumOutputs returns an unexpected result {} with error code {}", StringExt::GetErrorStr(res).c_str(), res);
                    break;
                }
                DXGI_OUTPUT_DESC output_desc{};
                res = output->GetDesc(&output_desc);
                if (res == S_OK) {
                    auto dev_name = StringExt::ToUTF8(output_desc.DeviceName);
                    monitors_.insert({dev_name, CaptureMonitorInfo{
                        .name_ = dev_name,
                        .attached_desktop_ = (bool) output_desc.AttachedToDesktop,
                        .top_ = output_desc.DesktopCoordinates.top,
                        .left_ = output_desc.DesktopCoordinates.left,
                        .right_ = output_desc.DesktopCoordinates.right,
                        .bottom_ = output_desc.DesktopCoordinates.bottom,
                        .supported_res_ = GetSupportedResolutions(output_desc.DeviceName),
                        .adapter_uid_ = adapter_uid,
                    }});
                    monitor_index++;
                } else {
                    LOGE("Failed to get output description of device: {} in adapter index: {}, adaper id: {}",
                         monitor_index, adapter_index, adapter_uid);
                    break;
                }
            } while(true);
            
            adapter_index++;
            
        } while(true);

        CalculateVirtualDeskInfo();

        LOGI("Finally, we got monitor size: {}", monitors_.size());
        for(const auto&[dev_name, monitor_info] : monitors_) {
            LOGI("In adapter:{}, the monitor:[{}]=>{}", monitor_info.adapter_uid_, dev_name, monitors_[dev_name].Dump());
        }
    }

    void DDACapturePlugin::InitCursorCapture() {
        cursor_capture_thread_ = std::make_shared<Thread>([=, this]() {
            cursor_capture_ = std::make_shared<CursorCapture>(this);
            while (!destroyed_) {
                cursor_capture_->Capture();
                auto target_duration = 1000 / capture_fps_;
                std::this_thread::sleep_for(std::chrono::milliseconds(target_duration));
            }
        }, "", false);
    }

    bool DDACapturePlugin::OnDestroy() {
        GrMonitorCapturePlugin::OnDestroy();
        if (cursor_capture_thread_ && cursor_capture_thread_->IsJoinable()) {
            cursor_capture_thread_->Join();
        }
        return true;
    }

    bool DDACapturePlugin::IsWorking() {
        return !captures_.empty() && init_success_;
    }

    bool DDACapturePlugin::StartCapturing() {
        for(const auto&[dev_name, monitor_info] : monitors_) {
            auto capture = std::make_shared<DDACapture>(this, monitor_info);
            if (!capture->Init()) {
                LOGE("dda capture init failed for target: {}", dev_name);
                continue;
            }
            capture->StartCapture();
            captures_.insert({dev_name, capture});
        }
        init_success_ = true;
        return init_success_;
    }

    void DDACapturePlugin::StopCapturing() {
        init_success_ = false;
    }

    std::vector<CaptureMonitorInfo> DDACapturePlugin::GetCaptureMonitorInfo() {
        if (!IsWorking()) {
            return {};
        }
        return sorted_monitors_;
    }

    std::string DDACapturePlugin::GetCapturingMonitorName() {
        return capturing_monitor_name_;
    }

    void DDACapturePlugin::SetCaptureMonitor(const std::string& name) {
        // todo: capture all monitors at same time
        if (IsWorking()) {
            capturing_monitor_name_ = name;
            if (name == "all") {
                // TODO
                for (const auto& [monitor_name, capture]: captures_) {
                    capture->ResumeCapture();
                }
            }
            else {
                for (const auto &[monitor_name, capture]: captures_) {
                    if (monitor_name == name) {
                        capture->ResumeCapture();
                    } else {
                        capture->PauseCapture();
                    }
                }
            }
        }

        NotifyCaptureMonitorInfo();
    }

    void DDACapturePlugin::SetCaptureFps(int fps) {
        GrMonitorCapturePlugin::SetCaptureFps(fps);
        if (IsWorking()) {
            for (const auto& [dev_name, capture] : captures_) {
                capture->SetCaptureFps(fps);
            }
        }
    }

    void DDACapturePlugin::On1Second() {
        NotifyCaptureMonitorInfo();
    }

    void DDACapturePlugin::OnNewClientIn() {
        GrPluginInterface::OnNewClientIn();
#if WIN32
        LOGI("OnNewClientIn will refresh desktop.");
        //SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, nullptr, SPIF_SENDCHANGE);
        //SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
        HWND desktop = GetDesktopWindow();
        if (InvalidateRect(desktop, NULL, TRUE)) {
            UpdateWindow(desktop);
        }
#endif
    }

    std::vector<SupportedResolution> DDACapturePlugin::GetSupportedResolutions(const std::wstring& name) {
        std::vector<SupportedResolution> resolutions;
        DEVMODE dm;
        dm.dmSize = sizeof(dm);
        dm.dmDriverExtra = 0;
        int mode_num = 0;
        while (EnumDisplaySettingsExW(name.c_str(), mode_num, &dm, 0)) {
            mode_num++;
            bool found = false;
            for (auto& res : resolutions) {
                if (res.width_ == dm.dmPelsWidth && res.height_ == dm.dmPelsHeight) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                resolutions.push_back(SupportedResolution{
                        .width_ = dm.dmPelsWidth,
                        .height_ = dm.dmPelsHeight,
                });
            }
        }
        return resolutions;
    }

    void DDACapturePlugin::CalculateVirtualDeskInfo() {
        sorted_monitors_.clear();
        int total_width = 0;
        int max_height = 0;
        int max_virtual_coord = 65535;
        for (auto& [idx, info] : monitors_) {
            sorted_monitors_.push_back(info);
            total_width += info.Width();
            if (info.Height() > max_height) {
                max_height = info.Height();
            }
            LOGI("ORIGIN, idx: {}, left: {}", idx, info.left_);
        }

        std::sort(sorted_monitors_.begin(), sorted_monitors_.end(), [](const CaptureMonitorInfo& lh, const CaptureMonitorInfo& rh) -> bool {
            return lh.left_ < rh.left_;
        });
        int left_monitor_virtual_size = 0;
        for (auto& info : sorted_monitors_) {
            info.virtual_width_ = info.Width() * 1.0f / total_width * max_virtual_coord;
            info.virtual_left_ = left_monitor_virtual_size;
            info.virtual_right_ = info.virtual_left_ + info.virtual_width_;
            left_monitor_virtual_size = info.virtual_right_;

            info.virtual_height_ = info.Height() * 1.0f / max_height * max_virtual_coord;
            info.virtual_top_ = info.top_ * 1.0f / max_height * max_virtual_coord;
            info.virtual_bottom_ = info.bottom_ * 1.0f / max_height * max_virtual_coord;

            LOGI("SORTED {}, left: {}, right: {}, top: {}, bottom: {}, \n "
                 "virtual width: {}, virtual height: {}, virtual left: {}, virtual right: {}, virtual top: {}, virtual bottom: {}, virtual h diff: {}",
                 info.name_, info.left_, info.right_, info.top_, info.bottom_,
                 info.virtual_width_, info.virtual_height_, info.virtual_left_, info.virtual_right_, info.virtual_top_, info.virtual_bottom_,
                 info.virtual_bottom_ - info.virtual_top_);
        }
    }

    void DDACapturePlugin::NotifyCaptureMonitorInfo() {
        auto event = std::make_shared<GrPluginCapturingMonitorInfoEvent>();
        this->CallbackEvent(event);
    }
}

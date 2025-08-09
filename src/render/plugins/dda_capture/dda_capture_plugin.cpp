//
// Created RGAA on 15/11/2024.
//

#include <ShlObj_core.h>

#include <memory>
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
        return "DXGI";
    }

    std::string DDACapturePlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t DDACapturePlugin::GetVersionCode() {
        return 110;
    }

    std::string DDACapturePlugin::GetPluginDescription() {
        return "DXGI desktop duplication";
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
            LOGI("Adapter Index:{} Name:{}, Uid:{}", adapter_index, StringUtil::ToUTF8(adapter_desc.Description).c_str(), adapter_uid);
            res = D3D11CreateDevice(adapter1, D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                    nullptr, 0, D3D11_SDK_VERSION, &d3d11_device, &feature_level,
                                    &d3d11_device_context);
            if (res != S_OK || !d3d11_device) {
                LOGE("D3D11CreateDevice failed: {}", StringUtil::GetErrorStr(res).c_str());
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
                     StringUtil::GetErrorStr(res), res);
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
                    LOGE("IDXGIAdapter::EnumOutputs returns an unexpected result {} with error code {}", StringUtil::GetErrorStr(res).c_str(), res);
                    break;
                }
                if (res == S_OK) {
                    LOGI("EnumOutputs ok Adapter Index:{} Name:{}, Uid:{}", adapter_index, StringUtil::ToUTF8(adapter_desc.Description).c_str(), adapter_uid);
                }
                
                DXGI_OUTPUT_DESC output_desc{};
                res = output->GetDesc(&output_desc);
                if (res == S_OK) {
                    auto dev_name = StringUtil::ToUTF8(output_desc.DeviceName);
                    LOGI("EnumOutputs S_OK, name_: {}", dev_name);
                    LOGI("Adapter Index:{} Name:{}, Uid:{}", adapter_index, StringUtil::ToUTF8(adapter_desc.Description).c_str(), adapter_uid);
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
        return !captures_.empty();
    }

    bool DDACapturePlugin::ExistCaptureMonitor(const std::string& name) {
        for (const auto& [dev_name, monitor_info] : monitors_) {
            if (dev_name == name) {
                return true;
            }
        }
        return false;
    }

    bool DDACapturePlugin::StartCapturing() {
        GrMonitorCapturePlugin::StartCapturing();
        StopCapturing();
        if (capturing_monitor_name_ != kAllMonitorsNameSign && !capturing_monitor_name_.empty()) {
            if (!ExistCaptureMonitor(capturing_monitor_name_)) {
                capturing_monitor_name_ = "";
            }
        }

        for(const auto&[dev_name, monitor_info] : monitors_) {
            auto capture = std::make_shared<DDACapture>(this, monitor_info);
            LOGI("DDACapturePlugin capture_fps_: {}", capture_fps_);
            capture->SetCaptureFps(capture_fps_);
            capture->StartCapture();
            capture->SetDDAInitCallback([=, this](bool init_res) {
                if (!init_res) {
                    if (capture_init_failed_cbk_) {
                        capture_init_failed_cbk_();
                    }
                    return;
                }

                if (kAllMonitorsNameSign == capturing_monitor_name_) {
                    capture->ResumeCapture();
                }
                else if(capturing_monitor_name_.empty()) {
                    if (capture->IsPrimaryMonitor()) {
                        capture->ResumeCapture();
                    }
                    capturing_monitor_name_ = capture->GetMyMonitorInfo().name_;
                }
                else if (capture->GetMyMonitorInfo().name_ == capturing_monitor_name_) {
                    capture->ResumeCapture();
                }
                SetCaptureMonitor(capturing_monitor_name_);
                NotifyCaptureMonitorInfo();
            });

            captures_.insert({dev_name, capture});
        }
        return true;
    }

    void DDACapturePlugin::StopCapturing() {
        for(const auto&[dev_name, capture] : captures_) {
            capture->StopCapture();
        }
    }

    void DDACapturePlugin::RestartCapturing() {
        LOGI("DDACapturePlugin RestartCapturing");
        StopCapturing();
        captures_.clear();
        monitors_.clear();
        InitVideoCaptures();
        StartCapturing();
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
        bool use_default_monitor = false;
        if (name.empty()) {
            use_default_monitor = true;
        }
        LOGI("SetCaptureMonitor: {}, use_default_monitor: {}", name, use_default_monitor);

        // todo: capture all monitors at same time
        if (IsWorking()) {
            if (kAllMonitorsNameSign == name) {
                capturing_monitor_name_ = name;
                // TODO
                for (const auto& [monitor_name, capture]: captures_) {
                    if (!capture->IsInitSuccess()) {
                        LOGW("Capture for: {} is not valid now.", monitor_name);
                        continue;
                    }
                    capture->ResumeCapture();
                }
            }
            else {
                for (const auto &[monitor_name, capture]: captures_) {
                    if (!name.empty()) {
                        if (monitor_name == name) {
                            capturing_monitor_name_ = name;
                            capture->ResumeCapture();
                        }
                        else {
                            capture->PauseCapture();
                        }
                    }
                    else {
                        if (!capture->IsInitSuccess()) {
                            LOGW("Capture for: {} is not valid now.", monitor_name);  // 如果StartCapturing后，接着执行SetCaptureMonitor，这时候 capture->IsInitSuccess () 返回 false
                            continue;
                        }
                        if (use_default_monitor && capture->IsPrimaryMonitor()) {
                            LOGI("Use default monitor: {}", monitor_name);
                            capturing_monitor_name_ = monitor_name;
                            capture->ResumeCapture();
                        }
                        else {
                            capture->PauseCapture();
                        }
                    }
                }
            }
        }

        bool has_resumed_capture = false;
        for (const auto &[monitor_name, capture]: captures_) {
            if (!capture->IsPausing()) {
                has_resumed_capture = true;
            }
        }
        if (!has_resumed_capture) {
            LOGW("Don't has resumed capture for: {}", name);
        }
        //LOGI("Capturing monitor name: {}", capturing_monitor_name_);
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
        // TODO: IGNORE THIS
        //SetCaptureMonitor(capturing_monitor_name_);
        //NotifyCaptureMonitorInfo();
        //LOGI("Capturing monitor: {}", capturing_monitor_name_);
        //for (auto& [k, v] : captures_) {
        //    LOGI("capture name: {}, fps: {}", k, v->GetCapturingFps());
        //}
    }

    void DDACapturePlugin::OnNewClientConnected(const std::string& visitor_device_id, const std::string& stream_id, const std::string& conn_type) {
        GrPluginInterface::OnNewClientConnected(visitor_device_id, stream_id, conn_type);
        for (const auto& [k, capture] : captures_) {
            capture->RefreshScreen();
            capture->TryWakeOs();
        }
        LOGI("OnNewClientConnected!");
        NotifyCaptureMonitorInfo();
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

		// to do 未测试0显示器的时候
        if (sorted_monitors_.size() <= 0) {
            return;
        }

        int far_left = sorted_monitors_[0].left_, far_top = sorted_monitors_[0].top_, far_right = sorted_monitors_[0].right_, far_bottom = sorted_monitors_[0].bottom_;

        int left_monitor_virtual_size = 0;
        for (auto& info : sorted_monitors_) {

            if (info.left_ < far_left) {
                far_left = info.left_;
            }

            if (info.top_ < far_top) {
                far_top = info.top_;
            }

            if (info.right_ > far_right) {
                far_right = info.right_;
            }

            if (info.bottom_ > far_bottom) {
                far_bottom = info.bottom_;
            }
        }
        virtual_desktop_bound_rectangle_info_.far_left_ = far_left;
        virtual_desktop_bound_rectangle_info_.far_top_ = far_top;
        virtual_desktop_bound_rectangle_info_.far_right_ = far_right;
        virtual_desktop_bound_rectangle_info_.far_bottom_ = far_bottom;
        LOGI("{}", virtual_desktop_bound_rectangle_info_.Dump());
    }

    void DDACapturePlugin::NotifyCaptureMonitorInfo() {
        auto event = std::make_shared<GrPluginCapturingMonitorInfoEvent>();
        this->CallbackEvent(event);
    }

    std::optional<int> DDACapturePlugin::GetMonIndexByName(const std::string& name) {
        int mon_index = 0;
        for (const auto& monitor : sorted_monitors_) {
            if (name == monitor.name_) {
                return { mon_index };
            }
            ++mon_index;
        }
        return { std::nullopt };
    }

    void DDACapturePlugin::DispatchAppEvent(const std::shared_ptr<AppBaseEvent>& event) {
        GrPluginInterface::DispatchAppEvent(event);
        //LOGI("DDACapturePlugin DispatchAppEvent type: {}", static_cast<int>(event->type_));
        if (!event) {
            return;
        }
        switch (event->type_)
        {
        case AppBaseEvent::EType::kDisplayDeviceChange: {
            LOGI("DDACapturePlugin DispatchAppEvent is kDisplayDeviceChange");
            HandleDisplayDeviceChangeEvent();
            break;
        }
        default:
            break;
        }
    }

    void DDACapturePlugin::HandleDisplayDeviceChangeEvent() {
        RestartCapturing();
    }

    VirtualDesktopBoundRectangleInfo DDACapturePlugin::GetVirtualDesktopBoundRectangleInfo() {
        return virtual_desktop_bound_rectangle_info_;
    }

    std::map<std::string, WorkingCaptureInfoPtr> DDACapturePlugin::GetWorkingCapturesInfo() {
        std::map<std::string, WorkingCaptureInfoPtr> result;
        for (const auto& [name, capture] : captures_) {
            if (capture->IsPausing()) {
                continue;
            }
            const auto& my_monitor = capture->GetMyMonitorInfo();
            result.insert({name, std::make_shared<WorkingCaptureInfo>(WorkingCaptureInfo {
                .target_name_ = name,
                .fps_ = capture->GetCapturingFps(),
                .capture_type_ = kCaptureTypeDXGI,
                .capture_frame_width_ = my_monitor.Width(),
                .capture_frame_height_ = my_monitor.Height(),
                .capture_gaps_ = capture->GetCaptureGaps(),
            })});
        }
        return result;
    }
}

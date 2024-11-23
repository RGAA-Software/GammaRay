#include "dda_capture.h"
#include <algorithm>
#include <iostream>
#include <timeapi.h>
#include <functional>
#include "tc_common_new/string_ext.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/log.h"
#include "tc_common_new/time_ext.h"
#include "tc_capture_new/capture_message.h"
#include "cursor_capture.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "Winmm.lib")

namespace tc
{

    DDACapture::DDACapture(DDACapturePlugin* plugin, const std::string& monitor)
        : DesktopCapture(plugin, monitor) {
        cursor_capture_ = std::make_shared<CursorCapture>(plugin);
        LOGI("DDACapture target monitor: {}", monitor);
    }

    DDACapture::~DDACapture() {
    }

    bool DDACapture::Init() {
        HRESULT res = 0;
        int adapter_index = 0;
        monitors_.clear();
        win_monitors_.clear();
        monitor_frame_index_.clear();
        res = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **) &factory1_);
        if (res != S_OK) {
            LOGE("CreateDXGIFactory1 failed");
            return false;
        }
        res = factory1_->EnumAdapters1(adapter_index, &adapter1_);
        if (res != S_OK) {
            LOGE("EnumAdapters1 failed, index: {}", adapter_index);
            return false;
        }
        D3D_FEATURE_LEVEL feature_level;
        DXGI_ADAPTER_DESC adapter_desc{};
        adapter1_->GetDesc(&adapter_desc);
        LOGI("Adapter Index:{} Name:{}", adapter_index, StringExt::ToUTF8(adapter_desc.Description).c_str());
        adapter_uid_ = adapter_desc.AdapterLuid.LowPart;
        res = D3D11CreateDevice(adapter1_, D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                nullptr, 0, D3D11_SDK_VERSION, &d3d11_device_, &feature_level, &d3d11_device_context_);
        if (res != S_OK || !d3d11_device_) {
            LOGE("D3D11CreateDevice failed: {}", StringExt::GetErrorStr(res).c_str());
            return false;
        }
        if (feature_level < D3D_FEATURE_LEVEL_11_0) {
            LOGE("D3D11CreateDevice returns an instance without DirectX 11 support, level : {}  Following initialization may fail", (int) feature_level);
        }
        CComPtr<IDXGIDevice> dxgi_device;
        res = d3d11_device_.QueryInterface(&dxgi_device);
        if (res != S_OK || !dxgi_device) {
            LOGE("ID3D11Device is not an implementation of IDXGIDevice, this usually means the system does not support DirectX 11. Error:{}, code: {}",
                 StringExt::GetErrorStr(res), res);
            return false;
        }
        monitor_count_ = GetSystemMetrics(SM_CMONITORS);
        dxgi_output_duplication_.clear();
        for (int i = 0; i < monitor_count_; ++i) {
            DXGIOutputDuplication duplication{};
            dxgi_output_duplication_[i] = duplication;
        }
        win_monitors_ = EnumerateAllMonitors();

        for (int index = 0; index < monitor_count_; ++index) {
            CComPtr<IDXGIOutput> output;
            res = adapter1_->EnumOutputs(index, &output);
            if (res == DXGI_ERROR_NOT_FOUND) {
                LOGE("adapter1_->EnumOutputs return DXGI_ERROR_NOT_FOUND,Please Check RDP connect.");
                break;
            }
            if (res == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) {
                LOGE("IDXGIAdapter::EnumOutputs returns NOT_CURRENTLY_AVAILABLE. This may happen when running in session 0");
                break;
            }
            if (res != S_OK || !output) {
                LOGE("IDXGIAdapter::EnumOutputs returns an unexpected result {} with error code {}",
                     StringExt::GetErrorStr(res).c_str(), res);
                continue;
            }
            DXGI_OUTPUT_DESC output_desc{};
            res = output->GetDesc(&output_desc);
            if (res == S_OK) {
                auto dev_name = StringExt::ToUTF8(output_desc.DeviceName);
                monitors_.insert({index, CaptureMonitorInfo {
                    .index_ = (MonitorIndex)index,
                    .name_ = dev_name,
                    .attached_desktop_ = (bool)output_desc.AttachedToDesktop,
                    .top_ = output_desc.DesktopCoordinates.top,
                    .left_ = output_desc.DesktopCoordinates.left,
                    .right_ = output_desc.DesktopCoordinates.right,
                    .bottom_ = output_desc.DesktopCoordinates.bottom,
                    .supported_res_ = GetSupportedResolutions(output_desc.DeviceName),
                }});
                dxgi_output_duplication_[index].output_desc_ = output_desc;
                dxgi_output_duplication_[index].monitor_win_info_ = monitors_[index];
                LOGI("=> {}", monitors_[index].Dump());

                if (dev_name == capturing_monitor_name_) {
                    capturing_monitor_index_ = index;
                    LOGI("Capture monitor index: {}, name: {}", capturing_monitor_index_, capturing_monitor_name_);
                }

                auto func_valid_rect = [](const RECT &rect) -> bool {
                    return rect.right > rect.left && rect.bottom > rect.top;
                };

                if (output_desc.AttachedToDesktop && func_valid_rect(output_desc.DesktopCoordinates)) {
                    CComPtr<IDXGIOutput1> output1;
                    res = output.QueryInterface(&output1);
                    if (res != S_OK || !output1) {
                        LOGE("Failed to convert IDXGIOutput to IDXGIOutput1, this usually means the system does not support DirectX 11");
                        continue;
                    }
                    static const int max_retry_count = 5;
                    for (int j = 0; j < max_retry_count; ++j) {
                        HRESULT error = output1->DuplicateOutput(d3d11_device_, &(dxgi_output_duplication_[index].duplication_));
                        if (error != S_OK || !dxgi_output_duplication_[index].duplication_) {
                            if (error == E_UNEXPECTED) {
                                LOGE("DuplicateOutput E_UNEXPECTED");
                            }
                            if (error == E_ACCESSDENIED) {
                                const ACCESS_MASK ac = GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE;
                                HDESK desk = ::OpenInputDesktop(0, 0, ac);
                                if (desk) {
                                    SetThreadDesktop(desk);
                                }
                                CloseDesktop(desk);
                                continue;
                            }
                            LOGE("Failed to duplicate output from IDXGIOutput1, error {} with code {}",
                                   StringExt::GetErrorStr(error).c_str(), error);
                            //return false;
                            continue;
                        } else {
                            LOGI("Init DDA mode success: {}", index);
                            //return true;
                            break;
                        }
                    }
                } else {
                    std::stringstream ss;
                    ss << (output_desc.AttachedToDesktop ? "Attached" : "Detached")
                       << " output " << index << " ("
                       << output_desc.DesktopCoordinates.top << ", "
                       << output_desc.DesktopCoordinates.left << ") - ("
                       << output_desc.DesktopCoordinates.bottom << ", "
                       << output_desc.DesktopCoordinates.right << ") is ignored.";
                    LOGI("MonitorInfo: {}", ss.str());
                }
            } else {
                LOGE("Failed to get output description of device :{}", index);
            }
        }
        if (!dxgi_output_duplication_[0].duplication_) {
            return false;
        }
        for (const auto& [idx, _]: dxgi_output_duplication_) {
            SharedD3d11Texture2D shared_texture;
            last_list_texture_[idx] = shared_texture;
        }

        CalculateVirtualDeskInfo();
        SendCapturingMonitorMessage();
        LOGI("Init DDA successfully.");
        return true;
    }

    bool DDACapture::Exit() {
        //for (auto& [idx, tex] : last_list_texture_) {
        //    if (tex.texture2d_) {
        //        tex.texture2d_->Release();
        //    }
        //}
        last_list_texture_.clear();
        for (auto& [idx, dp]: dxgi_output_duplication_) {
            if (dp.duplication_) {
                dp.duplication_->ReleaseFrame();
            }
            dp.duplication_.Release();
        }
        dxgi_output_duplication_.clear();
        d3d11_device_.Release();
        d3d11_device_context_.Release();
        dxgi_output_.Release();
        adapter1_.Release();
        factory1_.Release();
        return true;
    }

    DDACapture::CaptureResult DDACapture::CaptureNextFrame(int wait_time, CComPtr<ID3D11Texture2D>& out_tex, int mon_idx) {
        DXGI_OUTDUPL_FRAME_INFO info;
        CComPtr<IDXGIResource> resource;
        CComPtr<ID3D11Texture2D> source;
        HRESULT res;
        CaptureResult ret = CaptureResult::kSuccess;
        if (!dxgi_output_duplication_[mon_idx].duplication_) {
            if (!Init()) {
                // todo:
                //msg_notifier_->SendAppMessage(CaptureInitFailedMessage{});
                std::this_thread::sleep_for(std::chrono::seconds(1));
                return CaptureResult::kTryAgain;
            }
            ret = CaptureResult::kReInit;
        }
        dxgi_output_duplication_[mon_idx].duplication_->ReleaseFrame();

        res = dxgi_output_duplication_[mon_idx].duplication_->AcquireNextFrame(wait_time, &info, &resource);
        if (res != S_OK) {
            if (res == DXGI_ERROR_WAIT_TIMEOUT) {
                return CaptureResult::kTryAgain;
            } else if (res == DXGI_ERROR_ACCESS_LOST) {
                LOGE("DXGI_ERROR_ACCESS_LOST");
                return CaptureResult::kReInit;
            } else if (res == DXGI_ERROR_INVALID_CALL) {
                printf("DXGI_ERROR_INVALID_CALL");
                return CaptureResult::kReInit;
            }
            return CaptureResult::kTryAgain;
        }
        res = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void **) &source);
        if (res != S_OK) {
            LOGE("QueryInterface failed when capturing: {}", StringExt::GetErrorStr(res));
            return CaptureResult::kFailed;
        }
        if (info.AccumulatedFrames == 0) {
            return CaptureResult::kTryAgain;
        }
        out_tex = source;
        return ret;
    }

    void DDACapture::Start() {
        capture_thread_ = std::thread([this] {
            Capture();
        });
    }

    bool DDACapture::IsTargetMonitor(int index) {
        for (const auto& [idx, dxgi_monitor] : monitors_) {
            if (idx != index) {
                continue;
            }

            if (dxgi_monitor.name_ == capturing_monitor_name_ && !capturing_monitor_name_.empty()) {
                capturing_monitor_index_ = idx;
                return true;
            }

            for (const auto& win_monitor : win_monitors_) {
                // to capture primary monitor when no monitor specified
                if (capturing_monitor_name_.empty()) {
                    if (win_monitor.is_primary_ && win_monitor.name_ == dxgi_monitor.name_) {
                        capturing_monitor_name_ = win_monitor.name_;
                        capturing_monitor_index_ = idx;
                        return true;
                    } else {
                        continue;
                    }
                }
            }
        }
        return false;
    }

    void DDACapture::Capture() {
        while (!stop_flag_) {
            auto target_duration = 1000 / capture_fps_;
            auto beg = (int64_t)TimeExt::GetCurrentTimestamp();
            for (uint8_t index = 0; index < monitor_count_; ++index) {
                if (!IsTargetMonitor(index)) {
                    continue;
                }
                CComPtr<ID3D11Texture2D> texture = nullptr;
                DDACapture::CaptureResult res = CaptureNextFrame(target_duration, texture, index);
                if (res == DDACapture::CaptureResult::kFailed) {
                    LOGE("CaptureNextFrame index = {} failed.", index);
                    continue;
                } else if (res == DDACapture::CaptureResult::kReInit) {
                    LOGE("CaptureNextFrame reinit, index = {}.", index);
                    Exit();
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    auto init_ok = Init();
                    if (!init_ok) {
                        // todo:
                        //msg_notifier_->SendAppMessage(CaptureInitFailedMessage{});
                        LOGE("DDA re-init failed!");
                    }
                    continue;
                } else if (res == DDACapture::CaptureResult::kTryAgain) {
                    if (refresh_screen_) {
                        refresh_screen_ = false;
                        if (cached_textures_.find(index) != cached_textures_.end()) {
                            texture = cached_textures_[index];
                            LOGI("Use cached texture!");
                        }
                    } else {
                        continue;
                    }
                }

                if (texture) {
                    //std::string dds_name = "frame_index_" + std::to_string(index);
                    //DebugOutDDS(texture, dds_name);
                    OnCaptureFrame(texture, index);
                }
            }

            if (cursor_capture_) {
                cursor_capture_->Capture();
            }
            int64_t end = TimeExt::GetCurrentTimestamp();
            int64_t used_time = end - beg;
            int64_t diff = target_duration - used_time;
            // todo : more precise delay function.
            //if (diff > 5) {
                //std::this_thread::sleep_for(std::chrono::milliseconds(diff));
            //}
        }
    }

    void DDACapture::OnCaptureFrame(ID3D11Texture2D *texture, uint8_t monitor_index) {
        HRESULT result;
        // input texture info
        D3D11_TEXTURE2D_DESC input_desc;
        texture->GetDesc(&input_desc);
        UINT input_width = input_desc.Width;
        UINT input_height = input_desc.Height;
        DXGI_FORMAT input_format = input_desc.Format;

        // shared texture info if exists
        UINT shared_width = 0;
        UINT shared_height = 0;
        DXGI_FORMAT shared_format = DXGI_FORMAT_UNKNOWN;
        auto shared_texture = last_list_texture_[monitor_index].texture2d_;
        if (shared_texture) {
            D3D11_TEXTURE2D_DESC shared_desc;
            shared_texture->GetDesc(&shared_desc);
            shared_width = shared_desc.Width;
            shared_height = shared_desc.Height;
            shared_format = shared_desc.Format;
        }

        bool texture_changed = (input_width != shared_width)
                || (input_height != shared_height)
                || (input_format != shared_format);

        if (texture_changed) {
            if (shared_texture) {
                shared_texture->Release();
                last_list_texture_[monitor_index].texture2d_ = nullptr;
            }
            D3D11_TEXTURE2D_DESC create_desc;
            ZeroMemory(&create_desc, sizeof(create_desc));
            create_desc.Format = input_format;
            create_desc.Width = input_width;
            create_desc.Height = input_height;
            create_desc.MipLevels = 1;
            create_desc.ArraySize = 1;
            create_desc.SampleDesc.Count = 1;
            create_desc.Usage = D3D11_USAGE_DEFAULT;
            create_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            create_desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

            result = d3d11_device_->CreateTexture2D(&create_desc, nullptr, &last_list_texture_[monitor_index].texture2d_);
            if (FAILED(result)) {
                LOGE("desktop capture create texture failed with:{}", StringExt::GetErrorStr(result).c_str());
                return;
            }

            ComPtr<IDXGIResource> dxgiResource;
            result = last_list_texture_[monitor_index].texture2d_.As<IDXGIResource>(&dxgiResource);
            if (FAILED(result)) {
                LOGE("desktop capture as IDXGIResource failed with:{}", StringExt::GetErrorStr(result).c_str());
                return;
            }
            HANDLE handle;
            result = dxgiResource->GetSharedHandle(&handle);
            if (FAILED(result)) {
                LOGI("desktop capture get shared handle failed with:{}", StringExt::GetErrorStr(result).c_str());
                return;
            }
            last_list_texture_[monitor_index].shared_handle_ = handle;

            // cached textures
            if (cached_textures_.find(monitor_index) != cached_textures_.end()) {
                cached_textures_.erase(monitor_index);
            }
            CComPtr<ID3D11Texture2D> cached_texture;
            result = d3d11_device_->CreateTexture2D(&create_desc, nullptr, &cached_texture);
            if (FAILED(result)) {
                LOGE("create cached texture failed with:{}", StringExt::GetErrorStr(result).c_str());
                return;
            }
            cached_textures_[monitor_index] = cached_texture;
            LOGI("Create cached texture success: {}", monitor_index);
        }

        ComPtr<IDXGIKeyedMutex> keyMutex;
        result = last_list_texture_[monitor_index].texture2d_.As<IDXGIKeyedMutex>(&keyMutex);
        if (FAILED(result)) {
            LOGE("desktop frame capture as IDXGIKeyedMutex failed:{}", StringExt::GetErrorStr(result).c_str());
            return;
        }
        result = keyMutex->AcquireSync(0, INFINITE);
        if (FAILED(result)) {
            LOGE("desktop frame capture texture AcquireSync failed with:{}", StringExt::GetErrorStr(result).c_str());
            return;
        }

        d3d11_device_context_->CopyResource(last_list_texture_[monitor_index].texture2d_.Get(), texture);
        d3d11_device_context_->CopyResource(cached_textures_[monitor_index], texture);

        if (keyMutex) {
            keyMutex->ReleaseSync(0);
        }

        SendTextureHandle(last_list_texture_[monitor_index].shared_handle_, static_cast<MonitorIndex>(monitor_index), input_width, input_height, input_format);
    }

    void DDACapture::SendTextureHandle(const HANDLE &shared_handle, MonitorIndex monitor_index, uint32_t width, uint32_t height, DXGI_FORMAT format) {
        CaptureVideoFrame cap_video_frame{};
        cap_video_frame.type_ = kCaptureVideoFrame;
        cap_video_frame.capture_type_ = kCaptureVideoByHandle;
        cap_video_frame.data_length = 0;
        cap_video_frame.frame_width_ = width;
        cap_video_frame.frame_height_ = height;
        cap_video_frame.frame_index_ = GetFrameIndex(monitor_index);
        cap_video_frame.handle_ = reinterpret_cast<uint64_t>(shared_handle);
        cap_video_frame.frame_format_ = format;
        cap_video_frame.adapter_uid_ = adapter_uid_;
        cap_video_frame.monitor_index_ = static_cast<int8_t>(monitor_index);
        auto mon_win_info = dxgi_output_duplication_[monitor_index].monitor_win_info_;
        if (mon_win_info.Valid()) {
            memset(cap_video_frame.display_name_, 0, sizeof(cap_video_frame.display_name_));
            memcpy(cap_video_frame.display_name_, mon_win_info.name_.c_str(), mon_win_info.name_.size());
            cap_video_frame.left_ = mon_win_info.left_;
            cap_video_frame.top_ = mon_win_info.top_;
            cap_video_frame.right_ = mon_win_info.right_;
            cap_video_frame.bottom_ = mon_win_info.bottom_;
        }
        // todo:
        //msg_notifier_->SendAppMessage(cap_video_frame);
        LOGI("Captured...{}", cap_video_frame.handle_);
    }

    int DDACapture::GetFrameIndex(MonitorIndex monitor_index) {
        if (monitor_frame_index_.count(monitor_index) > 0) {
            monitor_frame_index_[monitor_index] = ++monitor_frame_index_[monitor_index];
        } else {
            monitor_frame_index_[monitor_index] = 0;
        }
        return monitor_frame_index_[monitor_index];
    }

    bool DDACapture::StartCapture() {
        this->Start();
        return true;
    }

    void DDACapture::StopCapture() {
        stop_flag_ = true;
        if (capture_thread_.joinable()) {
            capture_thread_.join();
        }
        this->Exit();
    }

    void DDACapture::CalculateVirtualDeskInfo() {
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

            LOGI("SORTED, idx: {}, left: {}, right: {}, top: {}, bottom: {}, \n "
                 "virtual width: {}, virtual height: {}, virtual left: {}, virtual right: {}, virtual top: {}, virtual bottom: {}, virtual h diff: {}",
                 info.index_, info.left_, info.right_, info.top_, info.bottom_,
                 info.virtual_width_, info.virtual_height_, info.virtual_left_, info.virtual_right_, info.virtual_top_, info.virtual_bottom_,
                 info.virtual_bottom_ - info.virtual_top_);
        }
    }

    std::vector<SupportedResolution> DDACapture::GetSupportedResolutions(const std::wstring& name) {
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

} // tc
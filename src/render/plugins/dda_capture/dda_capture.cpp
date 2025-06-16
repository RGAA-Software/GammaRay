#include "dda_capture.h"
#include <algorithm>
#include <iostream>
#include <timeapi.h>
#include <functional>
#include "tc_common_new/string_ext.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/log.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/monitors.h"
#include "tc_capture_new/capture_message.h"
#include "plugin_interface/gr_plugin_events.h"
#include "dda_capture_plugin.h"
#include "tc_common_new/win32/d3d_debug_helper.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "Winmm.lib")

#define S_NOT_CHANGED ((HRESULT)5L)

namespace tc
{

    DDACapture::DDACapture(DDACapturePlugin* plugin, const CaptureMonitorInfo& my_monitor_info)
        : PluginDesktopCapture(/*plugin, */my_monitor_info) {
        plugin_ = plugin;
        fps_stat_ = std::make_shared<FpsStat>();
        LOGI("DDACapture my monitor info: {}", my_monitor_info.Dump());
    }

    DDACapture::~DDACapture() {
    }

    bool DDACapture::Init() {
        HRESULT res = 0;
        int adapter_index = 0;

        CComPtr<IDXGIFactory1> factory1 = nullptr;

        res = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **) &factory1);
        if (res != S_OK) {
            LOGE("CreateDXGIFactory1 failed");
            return false;
        }
        do {
            CComPtr<IDXGIAdapter1> adapter1 = nullptr;
            CComPtr<ID3D11Device> d3d11_device = nullptr;
            CComPtr<ID3D11DeviceContext> d3d11_device_context = nullptr;

            res = factory1->EnumAdapters1(adapter_index, &adapter1);
            if (res != S_OK) {
                LOGE("EnumAdapters1 failed, index: {}", adapter_index);
                break;
            }
            D3D_FEATURE_LEVEL feature_level;
            DXGI_ADAPTER_DESC adapter_desc{};
            adapter1->GetDesc(&adapter_desc);
            LOGI("Adapter Index:{} Name:{}", adapter_index, StringExt::ToUTF8(adapter_desc.Description).c_str());
            auto adapter_uid = adapter_desc.AdapterLuid.LowPart;
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
                    LOGE("IDXGIAdapter::EnumOutputs returns an unexpected result {} with error code {}",
                         StringExt::GetErrorStr(res).c_str(), res);
                    monitor_index++;
                    continue;
                }
                DXGI_OUTPUT_DESC output_desc{};
                res = output->GetDesc(&output_desc);
                if (res == S_OK) {
                    auto dev_name = StringExt::ToUTF8(output_desc.DeviceName);
                    if (dev_name != my_monitor_info_.name_) {
                        LOGW("My device name is :{}, but your name is : {}, continue to find.", my_monitor_info_.name_, dev_name);
                        monitor_index++;
                        continue;
                    }
                    LOGI("Yes, found the same device: {}", dev_name);

                    dxgi_output_duplication_.output_desc_ = output_desc;
                    dxgi_output_duplication_.monitor_win_info_ = my_monitor_info_;

                    auto func_valid_rect = [](const RECT &rect) -> bool {
                        return rect.right > rect.left && rect.bottom > rect.top;
                    };

                    bool is_valid_rect = func_valid_rect(output_desc.DesktopCoordinates);
                    LOGI("AttachedToDesktop: {}, is valid rect: {}", output_desc.AttachedToDesktop, is_valid_rect);
                    if (output_desc.AttachedToDesktop && is_valid_rect) {
                        CComPtr<IDXGIOutput1> output1;
                        res = output.QueryInterface(&output1);
                        if (res != S_OK || !output1) {
                            LOGE("Failed to convert IDXGIOutput to IDXGIOutput1, this usually means the system does not support DirectX 11");
                            monitor_index++;
                            continue;
                        }

                        bool init_dda_success = false;
                        static const int max_retry_count = 5;
                        for (int j = 0; j < max_retry_count; ++j) {
                            HRESULT error = output1->DuplicateOutput(d3d11_device, &dxgi_output_duplication_.duplication_);
                            // to see : https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgioutput1-duplicateoutput
                            if (error != S_OK || !dxgi_output_duplication_.duplication_) {
                                if (error == E_UNEXPECTED) {
                                    LOGE("DuplicateOutput E_UNEXPECTED");
                                }
                                else if (error == E_ACCESSDENIED) {
                                    const ACCESS_MASK ac = GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE;
                                    HDESK desk = ::OpenInputDesktop(0, 0, ac);
                                    if (desk) {
                                        SetThreadDesktop(desk);
                                    }
                                    CloseDesktop(desk);
                                    continue;
                                }
                                else if (error == E_INVALIDARG) {
                                    LOGE("!! invalid args !!");
                                }
                                LOGE("Failed to duplicate output from IDXGIOutput1, error {} with code {:x}",
                                     StringExt::GetErrorStr(error).c_str(), (uint32_t) error);
                                continue;
                            } else {
                                init_dda_success = true;
                                LOGI("Init DDA mode success, monitor_index: {}, monitor_name: {}", monitor_index, my_monitor_info_.name_);
                                break;
                            }
                        }
                        if (init_dda_success) {
                            break;
                        }
                    } else {
                        std::stringstream ss;
                        ss << (output_desc.AttachedToDesktop ? "Attached" : "Detached")
                           << " output " << monitor_index << " ("
                           << output_desc.DesktopCoordinates.top << ", "
                           << output_desc.DesktopCoordinates.left << ") - ("
                           << output_desc.DesktopCoordinates.bottom << ", "
                           << output_desc.DesktopCoordinates.right << ") is ignored.";
                        LOGI("MonitorInfo invalid: {}", ss.str());
                    }
                    monitor_index++;
                } else {
                    LOGE("Failed to get output description of device :{}", monitor_index);
                }
            } while (true);

            if (!dxgi_output_duplication_.duplication_) {
                adapter_index++;
                continue;
            }
            last_list_texture_ = SharedD3d11Texture2D{};
            d3d11_device_ = d3d11_device;
            d3d11_device_context_ = d3d11_device_context;
            break;

        } while(true);

        if (!dxgi_output_duplication_.duplication_) {
            LOGI("Init DDA failed.");
            return false;
        }

        std::vector<MonitorWinInfo> win_monitors = EnumerateAllMonitors();
        for (const auto& info : win_monitors) {
            if (info.name_ == my_monitor_info_.name_ && info.is_primary_) {
                is_primary_monitor_ = true;
            }
        }

        LOGI("Init DDA successful");
        return true;
    }

    bool DDACapture::IsInitSuccess() {
        return dxgi_output_duplication_.duplication_ != nullptr;
    }

    bool DDACapture::Exit() {
        if (dxgi_output_duplication_.duplication_) {
            dxgi_output_duplication_.duplication_->ReleaseFrame();
            dxgi_output_duplication_.duplication_.Release();
        }
        d3d11_device_.Release();
        d3d11_device_context_.Release();
        return true;
    }

    HRESULT DDACapture::CaptureNextFrame(int wait_time, CComPtr<ID3D11Texture2D>& out_tex) {
        DXGI_OUTDUPL_FRAME_INFO info;
        CComPtr<IDXGIResource> resource;
        CComPtr<ID3D11Texture2D> source;
        HRESULT res;
        if (!dxgi_output_duplication_.duplication_) {
            if (!Init()) {
                LOGE("Capture dxgi init failed!");
                return S_FALSE;
            }
            if (!dxgi_output_duplication_.duplication_) {
                return S_FALSE;
            }
        }
        dxgi_output_duplication_.duplication_->ReleaseFrame();

        res = dxgi_output_duplication_.duplication_->AcquireNextFrame(wait_time, &info, &resource);
        if (res != S_OK) {
            return res;
        }
        res = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void **) &source);
        if (res != S_OK) {
            LOGE("QueryInterface failed when capturing: {}", StringExt::GetErrorStr(res));
            return res;
        }
        if (info.AccumulatedFrames == 0) {
            return S_NOT_CHANGED;
        }
        out_tex = source;
        return res;
    }

    void DDACapture::Start() {
        capture_thread_ = std::thread([this] {
            const int kInitTryMaxCount = 6;
            int try_count = -1;
            bool dda_init_res = false;

            do  { 
                ++try_count;
                dda_init_res = this->Init();
                if (!dda_init_res) {
                    LOGE("dda capture init failed for target: {}, will try again.", my_monitor_info_.name_);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }
                else {
                    break;
                }

            } while (try_count < kInitTryMaxCount);

            if (dda_init_callback_) {
                dda_init_callback_(dda_init_res);
            }

            if (dda_init_res) {
                LOGI("DDA Init success, will start capturing.");
                Capture();
            }
            else {
                LOGI("DDA Init failed, will use gdi capturing.");
            }
        });
    }

    void DDACapture::Capture() {
        while (!stop_flag_) {
            if (pausing_ || !d3d11_device_ || !d3d11_device_context_ /*|| plugin_->DontHaveConnectedClientsNow()*/) {
                std::this_thread::sleep_for(std::chrono::milliseconds(17));
                continue;
            }

            // test beg
            auto queuing_msg_count = plugin_->GetQueuingMediaMsgCountInNetPlugins();
            if (queuing_msg_count >= 10) {
                TimeUtil::DelayBySleep(1);
                LOGW("too many queuing messages, ignore this capturing loop, count: {}", queuing_msg_count);
                continue;
            }
            // test end

            uint64_t beg_time = TimeUtil::GetCurrentTimestamp();

            auto target_duration = 1000 / capture_fps_;
            //LOGI("target_duration: {}, capture_fps_: {}", target_duration, capture_fps_);
            CComPtr<ID3D11Texture2D> texture = nullptr;
            auto res = CaptureNextFrame(target_duration, texture);

            bool is_cached = false;
            if (res == S_OK) {
                // fps tick
                fps_stat_->Tick();

                // capture gaps
                auto curr_timestamp = (int64_t)TimeUtil::GetCurrentTimestamp();
                if (last_captured_timestamp_ == 0) {
                    last_captured_timestamp_ = curr_timestamp;
                }
                auto diff = curr_timestamp - last_captured_timestamp_;
                if (capture_gaps_.size() >= 180) {
                    capture_gaps_.pop_front();
                }
                capture_gaps_.push_back((int32_t)diff);
                last_captured_timestamp_ = curr_timestamp;
            }
            else if (res == S_FALSE || res == DXGI_ERROR_ACCESS_LOST || res == DXGI_ERROR_INVALID_CALL) {
                LOGE("CaptureNextFrame reinit, name = {}, err: {:x}, msg: {}", my_monitor_info_.name_, (uint32_t)res, StringExt::GetErrorStr(res));
                Exit();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                auto init_ok = Init();
                if (!init_ok) {
                    LOGE("ReInit failed!");
                } else {
                    used_cache_times_ = 0;
                    refresh_screen_ = true;
                    LOGE("ReInit successfully.");
                }
                continue;
            } else if ((res == DXGI_ERROR_WAIT_TIMEOUT || res == S_NOT_CHANGED)) {
                if (refresh_screen_) {
                    if (cached_texture_ == nullptr) {
                        continue;
                    }
                    if (used_cache_times_++ > 5) {
                        refresh_screen_ = false;
                    }
                    texture = cached_texture_;
                    is_cached = true;
                    LOGI("Use cached texture!");
                } else {
                    continue;
                }
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                LOGE("Unknown error: {:x}", res, StringExt::GetErrorStr(res));
                continue;
            }

            if (texture) {
                OnCaptureFrame(texture, is_cached);
                uint64_t end_time = TimeUtil::GetCurrentTimestamp();
                int cap_use_time = end_time - beg_time;
                if (target_duration > (cap_use_time + 5)) {
                    TimeUtil::DelayBySleep(target_duration - cap_use_time -3);
                    //LOGI("DelayBySleep: {}", target_duration - cap_use_time - 3);
                }
            }
        }
    }

    void DDACapture::OnCaptureFrame(ID3D11Texture2D *texture, bool is_cached) {
        HRESULT result;
        // input texture info
        D3D11_TEXTURE2D_DESC input_desc;
        texture->GetDesc(&input_desc);
        UINT input_width = input_desc.Width;
        UINT input_height = input_desc.Height;
        DXGI_FORMAT input_format = input_desc.Format;

        //LOGI("OnCaptureFrame texture, format: {}", (int)input_format); // DXGI_FORMAT_B8G8R8A8_UNORM

        // shared texture info if exists
        UINT shared_width = 0;
        UINT shared_height = 0;
        DXGI_FORMAT shared_format = DXGI_FORMAT_UNKNOWN;
        auto shared_texture = last_list_texture_.texture2d_;
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
            LOGI("texture changed, origin: {}x{}, format: {}", shared_width, shared_height, (int)shared_format);
            LOGI("texture changed, current: {}x{}, format: {}", input_width, input_height, (int)input_format);
            if (shared_texture) {
                shared_texture->Release();
                last_list_texture_.texture2d_ = nullptr;
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

            result = d3d11_device_->CreateTexture2D(&create_desc, nullptr, &last_list_texture_.texture2d_);
            if (FAILED(result)) {
                LOGE("desktop capture create texture failed with:{}", StringExt::GetErrorStr(result).c_str());
                return;
            }

            ComPtr<IDXGIResource> dxgiResource;
            result = last_list_texture_.texture2d_.As<IDXGIResource>(&dxgiResource);
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
            last_list_texture_.shared_handle_ = handle;

            // cached textures
            if (cached_texture_ != nullptr) {
                cached_texture_.Release();
                cached_texture_ = nullptr;
            }
            CComPtr<ID3D11Texture2D> cached_texture;
            result = d3d11_device_->CreateTexture2D(&create_desc, nullptr, &cached_texture);
            if (FAILED(result)) {
                LOGE("create cached texture failed with:{}", StringExt::GetErrorStr(result).c_str());
                return;
            }
            cached_texture_ = cached_texture;
            LOGI("Create cached texture success: {}", my_monitor_info_.name_);
        }

        ComPtr<IDXGIKeyedMutex> keyMutex;
        result = last_list_texture_.texture2d_.As<IDXGIKeyedMutex>(&keyMutex);
        if (FAILED(result)) {
            LOGE("desktop frame capture as IDXGIKeyedMutex failed:{}", StringExt::GetErrorStr(result).c_str());
            return;
        }
        result = keyMutex->AcquireSync(0, INFINITE);
        if (FAILED(result)) {
            LOGE("desktop frame capture texture AcquireSync failed with:{}", StringExt::GetErrorStr(result).c_str());
            return;
        }

        d3d11_device_context_->CopyResource(last_list_texture_.texture2d_.Get(), texture);
        if (!is_cached) {
            d3d11_device_context_->CopyResource(cached_texture_, texture);
        }

        if (keyMutex) {
            keyMutex->ReleaseSync(0);
        }

        SendTextureHandle(last_list_texture_.shared_handle_, input_width, input_height, input_format);
    }

    void DDACapture::SendTextureHandle(const HANDLE &shared_handle, uint32_t width, uint32_t height, DXGI_FORMAT format) {
        CaptureVideoFrame cap_video_frame{};
        cap_video_frame.type_ = kCaptureVideoFrame;
        cap_video_frame.capture_type_ = kCaptureVideoByHandle;
        cap_video_frame.data_length = 0;
        cap_video_frame.frame_width_ = width;
        cap_video_frame.frame_height_ = height;
        cap_video_frame.frame_index_ = GetFrameIndex();
        cap_video_frame.handle_ = reinterpret_cast<uint64_t>(shared_handle);
        cap_video_frame.frame_format_ = format;
        cap_video_frame.adapter_uid_ = my_monitor_info_.adapter_uid_;
        auto mon_index_res = plugin_->GetMonIndexByName(my_monitor_info_.name_);
        if (mon_index_res.has_value()) {
            cap_video_frame.monitor_index_ = mon_index_res.value();
        }
        else {
            LOGE("desktop capture get mon index by name failed!");
        }
        auto mon_win_info = dxgi_output_duplication_.monitor_win_info_;
        if (mon_win_info.Valid()) {
            memset(cap_video_frame.display_name_, 0, sizeof(cap_video_frame.display_name_));
            memcpy(cap_video_frame.display_name_, mon_win_info.name_.c_str(), mon_win_info.name_.size());
            cap_video_frame.left_ = mon_win_info.left_;
            cap_video_frame.top_ = mon_win_info.top_;
            cap_video_frame.right_ = mon_win_info.right_;
            cap_video_frame.bottom_ = mon_win_info.bottom_;
        }
        auto event = std::make_shared<GrPluginCapturedVideoFrameEvent>();
        event->frame_ = cap_video_frame;
        this->plugin_->CallbackEvent(event);

    }

    int DDACapture::GetFrameIndex() {
        monitor_frame_index_++;
        return monitor_frame_index_;
    }

    bool DDACapture::StartCapture() {
        this->Start();
        return true;
    }

    bool DDACapture::PauseCapture() {
        pausing_ = true;
        return true;
    }

    void DDACapture::ResumeCapture() {
        pausing_ = false;
        plugin_->InsertIdr();
    }

    void DDACapture::StopCapture() {
        stop_flag_ = true;
        if (capture_thread_.joinable()) {
            capture_thread_.join();
        }
        this->Exit();
    }

    void DDACapture::RefreshScreen() {
        PluginDesktopCapture::RefreshScreen();
        used_cache_times_ = 0;
    }

    bool DDACapture::IsPrimaryMonitor() {
        return is_primary_monitor_;
    }

    int DDACapture::GetCapturingFps() {
        return fps_stat_->value();
    }

} // tc
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
#include "plugin_interface/gr_plugin_events.h"
#include "dda_capture_plugin.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "Winmm.lib")

namespace tc
{

    DDACapture::DDACapture(DDACapturePlugin* plugin, const CaptureMonitorInfo& my_monitor_info)
        : DesktopCapture(plugin, my_monitor_info) {
        cursor_capture_ = std::make_shared<CursorCapture>(plugin);
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

                    if (output_desc.AttachedToDesktop && func_valid_rect(output_desc.DesktopCoordinates)) {
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
                                LOGI("Init DDA mode success: {}", monitor_index);
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
        LOGI("Init DDA successful");
        return true;
    }

    bool DDACapture::Exit() {
        if (dxgi_output_duplication_.duplication_) {
            dxgi_output_duplication_.duplication_->ReleaseFrame();
        }
        dxgi_output_duplication_.duplication_.Release();

        d3d11_device_.Release();
        d3d11_device_context_.Release();
        return true;
    }

    DDACapture::CaptureResult DDACapture::CaptureNextFrame(int wait_time, CComPtr<ID3D11Texture2D>& out_tex) {
        DXGI_OUTDUPL_FRAME_INFO info;
        CComPtr<IDXGIResource> resource;
        CComPtr<ID3D11Texture2D> source;
        HRESULT res;
        CaptureResult ret = CaptureResult::kSuccess;
        if (!dxgi_output_duplication_.duplication_) {
            if (!Init()) {
                // todo:
                //msg_notifier_->SendAppMessage(CaptureInitFailedMessage{});
                std::this_thread::sleep_for(std::chrono::seconds(1));
                return CaptureResult::kTryAgain;
            }
            ret = CaptureResult::kReInit;
        }
        dxgi_output_duplication_.duplication_->ReleaseFrame();

        res = dxgi_output_duplication_.duplication_->AcquireNextFrame(wait_time, &info, &resource);
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

    void DDACapture::Capture() {
        while (!stop_flag_) {
            if (pausing_ || !d3d11_device_ || !d3d11_device_context_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(17));
                continue;
            }

            auto target_duration = 1000 / capture_fps_;
            CComPtr<ID3D11Texture2D> texture = nullptr;
            DDACapture::CaptureResult res = CaptureNextFrame(target_duration, texture);
            if (res == DDACapture::CaptureResult::kFailed) {
                LOGE("CaptureNextFrame failed: {}", my_monitor_info_.name_);
                continue;
            } else if (res == DDACapture::CaptureResult::kReInit) {
                LOGE("CaptureNextFrame reinit, name = {}.", my_monitor_info_.name_);
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
                    if (cached_texture_ != nullptr) {
                        texture = cached_texture_;
                        LOGI("Use cached texture!");
                    }
                } else {
                    continue;
                }
            }

            if (texture) {
                //std::string dds_name = "frame_index_" + std::to_string(index);
                //DebugOutDDS(texture, dds_name);
                OnCaptureFrame(texture);
            }

            if (cursor_capture_) {
                cursor_capture_->Capture();
            }
        }
    }

    void DDACapture::OnCaptureFrame(ID3D11Texture2D *texture) {
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
        d3d11_device_context_->CopyResource(cached_texture_, texture);

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


} // tc
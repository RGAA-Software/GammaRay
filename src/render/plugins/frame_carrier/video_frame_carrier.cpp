//
// Created by RGAA on 19/11/2024.
//

#include "video_frame_carrier.h"
#include <atlcomcli.h>
#include <libyuv/convert.h>
#include <libyuv/convert_from_argb.h>
#include "tc_common_new/log.h"
#include "tc_common_new/string_util.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/image.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/defer.h"
#include "tc_common_new/win32/d3d_debug_helper.h"
#include "tc_common_new/file.h"
#include "frame_carrier_plugin.h"
#include "plugins/plugin_manager.h"
#include "plugin_interface/gr_frame_processor_plugin.h"

namespace tc
{

    VideoFrameCarrier::VideoFrameCarrier(FrameCarrierPlugin* plugin,
                                         const ComPtr<ID3D11Device>& d3d11_device,
                                         const ComPtr<ID3D11DeviceContext>& d3d11_device_context,
                                         uint64_t adapter_uid,
                                         const std::string& monitor_name,
                                         bool resize,
                                         int resize_width,
                                         int resize_height,
                                         bool enable_full_color_mode)
    {
        plugin_ = plugin;
        d3d11_device_ = d3d11_device;
        d3d11_device_context_ = d3d11_device_context;
        adapter_uid_ = adapter_uid;
        monitor_name_ = monitor_name;
        resize_ = resize;
        resize_width_ = resize_width;
        resize_height_ = resize_height;
        enable_full_color_mode_ = enable_full_color_mode;
        yuv_converter_thread_ = Thread::Make("video frame carrier", 1024);
        yuv_converter_thread_->Poll();
    }

    bool VideoFrameCarrier::D3D11Texture2DLockMutex(const ComPtr<ID3D11Texture2D>& texture2d) {
        HRESULT res;
        ComPtr<IDXGIKeyedMutex> key_mutex;
        res = texture2d.As<IDXGIKeyedMutex>(&key_mutex);
        if (FAILED(res)) {
            printf("D3D11Texture2DReleaseMutex IDXGIKeyedMutex. error\n");
            return false;
        }
        res = key_mutex->AcquireSync(0, INFINITE);
        if (FAILED(res)) {
            printf("D3D11Texture2DReleaseMutex AcquireSync failed.\n");
            return false;
        }
        return true;
    }

    bool VideoFrameCarrier::D3D11Texture2DReleaseMutex(const ComPtr<ID3D11Texture2D>& texture2d) {
        HRESULT res;
        ComPtr<IDXGIKeyedMutex> key_mutex;
        res = texture2d.As<IDXGIKeyedMutex>(&key_mutex);
        if (FAILED(res)) {
            printf("D3D11Texture2DReleaseMutex IDXGIKeyedMutex. error\n");
            return false;
        }
        res = key_mutex->ReleaseSync(0);
        if (FAILED(res)) {
            printf("D3D11Texture2DReleaseMutex ReleaseSync failed.\n");
            return false;
        }
        return true;
    }

    bool VideoFrameCarrier::CopyID3D11Texture2D(const ComPtr<ID3D11Texture2D>& shared_texture) {
        if (!D3D11Texture2DLockMutex(shared_texture)) {
            LOGE("D3D11Texture2DLockMutex error");
            return false;
        }
        std::shared_ptr<void> auto_release_texture2D_mutex((void *) nullptr, [=, this](void *temp) {
            D3D11Texture2DReleaseMutex(shared_texture);
        });

        HRESULT res;
        D3D11_TEXTURE2D_DESC desc;
        shared_texture->GetDesc(&desc);

        ComPtr<ID3D11Device> curDevice;
        shared_texture->GetDevice(&curDevice);

        if (texture2d_) {
            ComPtr<ID3D11Device> sharedTextureDevice;
            texture2d_->GetDevice(&sharedTextureDevice);
            if (sharedTextureDevice != curDevice) {
                texture2d_ = nullptr;
            }
            if (texture2d_) {
                D3D11_TEXTURE2D_DESC sharedTextureDesc;
                texture2d_->GetDesc(&sharedTextureDesc);
                if (desc.Width != sharedTextureDesc.Width ||
                    desc.Height != sharedTextureDesc.Height ||
                    desc.Format != sharedTextureDesc.Format) {
                    texture2d_ = nullptr;
                }
            }
        }

        if (!texture2d_) {
            D3D11_TEXTURE2D_DESC createDesc;
            ZeroMemory(&createDesc, sizeof(createDesc));
            createDesc.Format = desc.Format;
            createDesc.Width = desc.Width;
            createDesc.Height = desc.Height;
            createDesc.MipLevels = 1;
            createDesc.ArraySize = 1;
            createDesc.SampleDesc.Count = 1;
            //createDesc.Usage = D3D11_USAGE_DEFAULT;
            //createDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            createDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            createDesc.Usage = D3D11_USAGE_STAGING;
            res = curDevice->CreateTexture2D(&createDesc, NULL, texture2d_.GetAddressOf());
            if (FAILED(res)) {
                LOGE("desktop capture create texture failed with:{}", StringUtil::GetErrorStr(res).c_str());
                return false;
            }
        }
        ComPtr<ID3D11DeviceContext> ctx;
        curDevice->GetImmediateContext(&ctx);
        ctx->CopyResource(texture2d_.Get(), shared_texture.Get());

        return true;
    }

    ComPtr<ID3D11Texture2D> VideoFrameCarrier::OpenSharedTexture(HANDLE handle) {
        ComPtr<ID3D11Texture2D> sharedTexture;
        HRESULT res;
        res = d3d11_device_->OpenSharedResource(handle, IID_PPV_ARGS(sharedTexture.GetAddressOf()));
        if (FAILED(res)) {
            LOGE("OpenSharedResource failed: {:x}", (uint32_t)res);
            return nullptr;
        }
        return sharedTexture;
    }

    ID3D11Texture2D* VideoFrameCarrier::CopyTexture(const std::string& mon_name, uint64_t handle, uint64_t frame_index) {
        ComPtr<ID3D11Texture2D> shared_texture;
        shared_texture = OpenSharedTexture(reinterpret_cast<HANDLE>(handle));
        if (!shared_texture) {
            LOGE("OpenSharedTexture failed.");
            return nullptr;
        }

        auto frame_resize_plugin = plugin_->GetFrameResizePlugin(mon_name);
        if (!frame_resize_plugin && resize_) {
            LOGE("You want to resize, but there is not a resize plugin.");
            return nullptr;
        }

        D3D11_TEXTURE2D_DESC desc;
        shared_texture->GetDesc(&desc);
        if (resize_) {
            if (!D3D11Texture2DLockMutex(shared_texture)) {
                LOGE("D3D11Texture2DLockMutex error\n");
                return nullptr;
            }
            std::shared_ptr<void> auto_release_texture2D_mutex((void *) nullptr, [=, this](void *temp) {
                D3D11Texture2DReleaseMutex(shared_texture);
            });
            auto final_texture
                = frame_resize_plugin->Process(shared_texture.Get(), adapter_uid_, monitor_name_, resize_width_, resize_height_);
            if (!final_texture) {
                LOGE("Frame resize failed!");
                return nullptr;
            }
            //DebugOutDDS(final_texture.Get(), "3.dds");
            return final_texture.Get();
        } else {
            if (!CopyID3D11Texture2D(shared_texture)) {
                LOGE("CopyID3D11Texture2D failed.");
                return nullptr;
            }
            //DebugOutDDS(texture2d_.Get(), "2.dds");
            //PrintD3DTexture2DDesc("frame carrier, texture2d", texture2d_.Get());
            return texture2d_.Get();
        }
    }

    bool VideoFrameCarrier::ConvertRawImage(const std::shared_ptr<Image> image,
                                            std::function<void(const std::shared_ptr<Image>&)>&& rgba_cbk,
                                            std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk) {
        raw_image_rgba_ = image;
        rgba_cbk(raw_image_rgba_);
		// to do 明确下gdi原图的像素布局
        if (enable_full_color_mode_) {
            ConvertToYuv444(std::move(yuv_cbk));
        }
        else {
            ConvertToYuv420(std::move(yuv_cbk));
        }

        return true;
    }

    bool VideoFrameCarrier::MapRawTexture(ID3D11Texture2D* texture, DXGI_FORMAT format, int height,
                                          std::function<void(const std::shared_ptr<Image>&)>&& rgba_cbk,
                                          std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk) {
        CComPtr<IDXGISurface> staging_surface = nullptr;
        auto hr = texture->QueryInterface(IID_PPV_ARGS(&staging_surface));
        if (FAILED(hr)) {
            LOGE("MapRawTexture !QueryInterface(IDXGISurface) err");
            return false;
        }
        DXGI_MAPPED_RECT mapped_rect{};
        hr = staging_surface->Map(&mapped_rect, DXGI_MAP_READ);
        if (FAILED(hr)) {
            LOGE("MapRawTexture !Map(IDXGISurface)");
            return false;
        }
        auto defer = Defer::Make([staging_surface]() {
            staging_surface->Unmap();
        });

        // copy to raw image buffer
        raw_image_rgba_format_ = format;
        bool ok = CopyToRawImage(mapped_rect.pBits, mapped_rect.Pitch, height);
        if (ok) {
            rgba_cbk(raw_image_rgba_);
        }
        
        if (enable_full_color_mode_) {
            ConvertToYuv444(std::move(yuv_cbk));
        }
        else {
            ConvertToYuv420(std::move(yuv_cbk));
        }

        return ok;
    }

    bool VideoFrameCarrier::CopyToRawImage(const uint8_t* data, int row_pitch_bytes, int height) {
        auto total_size = row_pitch_bytes * height;
        auto width = row_pitch_bytes / 4;
        if (raw_image_rgba_ == nullptr || (raw_image_rgba_->GetData() && raw_image_rgba_->GetData()->Size() != total_size)) {
            raw_image_rgba_ = Image::Make(Data::Make(nullptr, total_size), width, height);
        }

        if (total_size > raw_image_rgba_->GetData()->Size()) {
            LOGE("raw image buffer is too small, you need to resize it!");
            return false;
        }
        memcpy(raw_image_rgba_->GetData()->DataAddr(), data, total_size);
        raw_image_rgba_->raw_img_type_ = (RawImageType)GetRawImageType();
        return true;
    }

    void VideoFrameCarrier::ConvertToYuv420(std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk) {
        auto task = [=, this]() {
            auto beg = TimeUtil::GetCurrentTimestamp();
            if (!raw_image_rgba_ || !raw_image_rgba_->GetData()) {
                return;
            }
            if (!raw_image_yuv_ ||
                (raw_image_yuv_->GetWidth() != raw_image_rgba_->GetWidth() || raw_image_yuv_->GetHeight() != raw_image_yuv_->GetHeight()) ||
                raw_image_yuv_->raw_img_type_ != RawImageType::kI420) 
            {
                raw_image_yuv_ = Image::Make(Data::Make(nullptr, raw_image_rgba_->GetWidth() * raw_image_rgba_->GetHeight() * 1.5),
                                             raw_image_rgba_->GetWidth(), raw_image_rgba_->GetHeight(), RawImageType::kI420);
            }
            int width = raw_image_rgba_->GetWidth();
            int height = raw_image_rgba_->GetHeight();
            size_t pixel_size = width * height;

            const int uv_stride = width >> 1;
            uint8_t* y = (uint8_t*)raw_image_yuv_->GetData()->DataAddr();
            uint8_t* u = y + pixel_size;
            uint8_t* v = u + (pixel_size >> 2);

            auto pitch = raw_image_rgba_->GetWidth() * 4;
            auto data_buffer = (uint8_t*)raw_image_rgba_->GetData()->DataAddr();
            if (DXGI_FORMAT_B8G8R8A8_UNORM == raw_image_rgba_format_) {
                libyuv::ARGBToI420(data_buffer, pitch, y, width, u, uv_stride, v, uv_stride, width, height);
            }
            else if (DXGI_FORMAT_R8G8B8A8_UNORM == raw_image_rgba_format_) {
                libyuv::ABGRToI420(data_buffer, pitch, y, width, u, uv_stride, v, uv_stride, width, height);
            }
            else {
                libyuv::ARGBToI420(data_buffer, pitch, y, width, u, uv_stride, v, uv_stride, width, height);
            }

#if 0   // save to file
            static int index = 0;
            auto file = File::OpenForWrite("ConvertToYuv_" +  std::to_string(index % 10) + ".yuv420");
            if (file) {
                file->Write(0, raw_image_yuv_->GetData());
            }
            ++index;
#endif

            yuv_cbk(raw_image_yuv_);
        };
        yuv_converter_thread_->Post(std::move(task));

    }

    void VideoFrameCarrier::ConvertToYuv444(std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk) {
        auto task = [=, this]() {
            auto beg = TimeUtil::GetCurrentTimestamp();

            if (!raw_image_rgba_ || !raw_image_rgba_->GetData()) {
                return;
            }

            if (!raw_image_yuv_ ||
                (raw_image_yuv_->GetWidth() != raw_image_rgba_->GetWidth() || raw_image_yuv_->GetHeight() != raw_image_yuv_->GetHeight()) ||
                raw_image_yuv_->raw_img_type_ != RawImageType::kI444)
            {
                raw_image_yuv_ = Image::Make(Data::Make(nullptr, raw_image_rgba_->GetWidth() * raw_image_rgba_->GetHeight() * 3),
                    raw_image_rgba_->GetWidth(), raw_image_rgba_->GetHeight(), RawImageType::kI444);
            }
            int width = raw_image_rgba_->GetWidth();
            int height = raw_image_rgba_->GetHeight();
            size_t pixel_size = width * height;

            const int uv_stride = width;
            uint8_t* y = (uint8_t*)raw_image_yuv_->GetData()->DataAddr();
            uint8_t* u = y + pixel_size;
            uint8_t* v = u + pixel_size;

            auto pitch = raw_image_rgba_->GetWidth() * 4;
            auto data_buffer = (uint8_t*)raw_image_rgba_->GetData()->DataAddr();
            if (DXGI_FORMAT_B8G8R8A8_UNORM == raw_image_rgba_format_) {
                libyuv::ARGBToI444(data_buffer, pitch, y, width, u, uv_stride, v, uv_stride, width, height);
            }
            else if (DXGI_FORMAT_R8G8B8A8_UNORM == raw_image_rgba_format_) {
                libyuv::ARGBToI444(data_buffer, pitch, y, width, u, uv_stride, v, uv_stride, width, height);
            }
            else {
                libyuv::ARGBToI444(data_buffer, pitch, y, width, u, uv_stride, v, uv_stride, width, height);
            }

#if 0   // save yuv file
            static int index = 0;
            auto yuv444_file = File::OpenForWrite("ConvertToYuv_" + std::to_string(index % 10) + ".yuv444");
            if (yuv444_file) {
                yuv444_file->Write(0, raw_image_yuv_->GetData());
            }
            ++index;
#endif
            yuv_cbk(raw_image_yuv_);
        };
        yuv_converter_thread_->Post(std::move(task));

    }

    int VideoFrameCarrier::GetRawImageType() const {
        if (DXGI_FORMAT_B8G8R8A8_UNORM == raw_image_rgba_format_) {
            return (int)RawImageType::kBGRA;
        }
        else if (DXGI_FORMAT_R8G8B8A8_UNORM == raw_image_rgba_format_) {
            return (int)RawImageType::kRGBA;
        }
        else {
            return (int)RawImageType::kRGBA;
        }
    }

    void VideoFrameCarrier::Exit() {
        if (yuv_converter_thread_) {
            yuv_converter_thread_->Exit();
        }
    }

    bool VideoFrameCarrier::IsResizeFrameSize() {
        return resize_;
    }

    int VideoFrameCarrier::GetResizeWidth() {
        return resize_width_;
    }

    int VideoFrameCarrier::GetResizeHeight() {
        return resize_height_;
    }

}
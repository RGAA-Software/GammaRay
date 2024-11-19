//
// Created by RGAA on 19/11/2024.
//

#ifndef GAMMARAY_VIDEO_FRAME_CARRIER_H
#define GAMMARAY_VIDEO_FRAME_CARRIER_H

#include <cstdint>
#include <functional>
#include <memory>
#ifdef WIN32
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#endif
#include <mutex>

namespace tc
{
    using namespace Microsoft::WRL;

    class Image;
    class Thread;
    class Context;
    class FrameRender;

    // move video frames from provider / capture
    class VideoFrameCarrier {
    public:
        explicit VideoFrameCarrier(const std::shared_ptr<Context>& ctx);

        bool D3D11Texture2DLockMutex(ComPtr<ID3D11Texture2D> texture2d);
        bool D3D11Texture2DReleaseMutex(ComPtr<ID3D11Texture2D> texture2d);
        bool CopyID3D11Texture2D(ComPtr<ID3D11Texture2D> shared_texture2d);
        ComPtr<ID3D11Texture2D> OpenSharedTexture(HANDLE handle);

    private:
        std::shared_ptr<Context> context_ = nullptr;
        ComPtr<ID3D11Device> d3d11_device_;
        ComPtr<ID3D11DeviceContext> d3d11_device_context_;
        ComPtr<ID3D11Texture2D> texture2d_;
        std::shared_ptr<FrameRender> frame_render_ = nullptr;

        std::mutex raw_image_rgba_mtx_;
        std::shared_ptr<Image> raw_image_rgba_ = nullptr;
        int raw_image_rgba_format_ = -1;
        std::mutex raw_image_yuv_mtx_;
        std::shared_ptr<Image> raw_image_yuv_ = nullptr;

        // async yuv converter
        std::shared_ptr<Thread> yuv_converter_thread_ = nullptr;
    };

}

#endif //GAMMARAY_VIDEO_FRAME_CARRIER_H

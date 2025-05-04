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
    class RdContext;
    class FrameRender;
    class GrFrameProcessorPlugin;
    class PluginManager;
    class RdSettings;

    // move video frames from provider / capture
    class VideoFrameCarrier {
    public:
        explicit VideoFrameCarrier(const std::shared_ptr<RdContext>& ctx,
                                   const ComPtr<ID3D11Device>& d3d11_device,
                                   const ComPtr<ID3D11DeviceContext>& d3d11_device_context,
                                   uint64_t adapter_uid,
                                   const std::string& monitor_name,
                                   bool resize,
                                   int resize_width,
                                   int resize_height);

        bool MapRawTexture(ID3D11Texture2D* texture, DXGI_FORMAT format, int height,
                           std::function<void(const std::shared_ptr<Image>&)>&& rgba_cbk,
                           std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk);

        ID3D11Texture2D* CopyTexture(uint64_t handle, uint64_t frame_index);
        void Exit();

    private:
        static bool D3D11Texture2DLockMutex(const ComPtr<ID3D11Texture2D>& texture2d);
        static bool D3D11Texture2DReleaseMutex(const ComPtr<ID3D11Texture2D>& texture2d);
        bool CopyID3D11Texture2D(const ComPtr<ID3D11Texture2D>& shared_texture2d);
        ComPtr<ID3D11Texture2D> OpenSharedTexture(HANDLE handle);
        bool CopyToRawImage(const uint8_t* data, int row_pitch_bytes, int height);
        void ConvertToYuv(std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk);
        [[nodiscard]] int GetRawImageType() const;

    private:
        RdSettings* settings_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        ComPtr<ID3D11Device> d3d11_device_;
        ComPtr<ID3D11DeviceContext> d3d11_device_context_;
        ComPtr<ID3D11Texture2D> texture2d_;
        std::shared_ptr<FrameRender> frame_render_ = nullptr;

        std::shared_ptr<Image> raw_image_rgba_ = nullptr;
        int raw_image_rgba_format_ = -1;
        std::shared_ptr<Image> raw_image_yuv_ = nullptr;

        // async yuv converter
        std::shared_ptr<Thread> yuv_converter_thread_ = nullptr;

        bool resize_ = false;
        int resize_width_ = 0;
        int resize_height_ = 0;

        std::shared_ptr<PluginManager> plugin_manager_ = nullptr;
        GrFrameProcessorPlugin* frame_resize_plugin_ = nullptr;

        uint64_t adapter_uid_ = 0;
        std::string monitor_name_;
    };

}

#endif //GAMMARAY_VIDEO_FRAME_CARRIER_H

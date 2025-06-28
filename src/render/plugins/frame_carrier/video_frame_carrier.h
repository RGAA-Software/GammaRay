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
#include "tc_common_new/image.h"

namespace tc
{
    using namespace Microsoft::WRL;

    class Image;
    class Thread;
    class FrameRender;
    class FrameCarrierPlugin;
    class GrFrameProcessorPlugin;

    // move video frames from provider / capture
    class VideoFrameCarrier {
    public:
        explicit VideoFrameCarrier(FrameCarrierPlugin* plugin,
                                   const ComPtr<ID3D11Device>& d3d11_device,
                                   const ComPtr<ID3D11DeviceContext>& d3d11_device_context,
                                   uint64_t adapter_uid,
                                   const std::string& monitor_name,
                                   bool resize,
                                   int resize_width,
                                   int resize_height, 
                                   bool enable_full_color_mode = false);

        bool MapRawTexture(ID3D11Texture2D* texture, DXGI_FORMAT format, int height,
                           std::function<void(const std::shared_ptr<Image>&)>&& rgba_cbk,
                           std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk);

        ID3D11Texture2D* CopyTexture(const std::string& mon_name, uint64_t handle, uint64_t frame_index);

        bool ConvertRawImage(const std::shared_ptr<Image> image,
                            std::function<void(const std::shared_ptr<Image>&)>&& rgba_cbk,
                            std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk);

        void Exit();

        bool IsResizeFrameSize();
        int GetResizeWidth();
        int GetResizeHeight();

    private:
        static bool D3D11Texture2DLockMutex(const ComPtr<ID3D11Texture2D>& texture2d);
        static bool D3D11Texture2DReleaseMutex(const ComPtr<ID3D11Texture2D>& texture2d);
        bool CopyID3D11Texture2D(const ComPtr<ID3D11Texture2D>& shared_texture2d);
        ComPtr<ID3D11Texture2D> OpenSharedTexture(HANDLE handle);
        bool CopyToRawImage(const uint8_t* data, int row_pitch_bytes, int height);
        void ConvertToYuv420(std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk);
        void ConvertToYuv444(std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk);
        [[nodiscard]] int GetRawImageType() const;

    private:
        FrameCarrierPlugin* plugin_ = nullptr;
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

        uint64_t adapter_uid_ = 0;
        std::string monitor_name_;

        bool enable_full_color_mode_ = false;
    };

}

#endif //GAMMARAY_VIDEO_FRAME_CARRIER_H

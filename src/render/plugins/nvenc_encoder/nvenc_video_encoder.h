//
// Created by RGAA  on 2024/1/6.
//

#ifndef TC_APPLICATION_NVENC_VIDEO_ENCODER_H
#define TC_APPLICATION_NVENC_VIDEO_ENCODER_H

#include "nvencoder/12/NvEncoderD3D11.h"
#include "tc_encoder_new/encoder_config.h"
#include <fstream>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <any>
#include "tc_common_new/fps_stat.h"

using namespace Microsoft::WRL;

namespace tc
{

    class FrameRender;
    class NvencEncoderPlugin;

    class NVENCVideoEncoder {
    public:
        NVENCVideoEncoder(NvencEncoderPlugin* plugin, uint64_t adapter_uid);
        ~NVENCVideoEncoder();

        bool Initialize(const tc::EncoderConfig& config);
        void Encode(ID3D11Texture2D* tex2d, uint64_t frame_index, std::any extra);
        void InsertIdr();
        void Exit();
        int32_t GetEncodeFps();

    private:
        void Transmit(ID3D11Texture2D* pTexture, uint64_t frame_index, std::any extra);
        void Shutdown();
        void FillEncodeConfig(NV_ENC_INITIALIZE_PARAMS& initialize_params, int refreshRate, int renderWidth, int renderHeight, uint64_t bitrate_bps);
        static NV_ENC_BUFFER_FORMAT DxgiFormatToNvEncFormat(DXGI_FORMAT dxgiFormat);

    private:
        std::shared_ptr<NvEncoder> nv_encoder_ = nullptr;
        EncoderConfig encoder_config_;
        bool insert_idr_ = false;
        tc::NvencEncoderPlugin* plugin_ = nullptr;

        ComPtr<ID3D11Device> d3d11_device_;
        ComPtr<ID3D11DeviceContext> d3d11_device_context_;
        ComPtr<ID3D11Texture2D> texture2d_;

        std::shared_ptr<FpsStat> fps_stat_ = nullptr;

    };

}
#endif //TC_APPLICATION_NVENC_VIDEO_ENCODER_H

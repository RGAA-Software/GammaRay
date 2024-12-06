//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_NVENC_ENCODER_PLUGIN_H
#define GAMMARAY_NVENC_ENCODER_PLUGIN_H

#include "plugin_interface/gr_encoder_plugin.h"

namespace tc
{

    class NVENCVideoEncoder;

    class NvencEncoderPlugin : public GrEncoderPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        void On1Second() override;
        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        void InsertIdr() override;
        bool IsWorking() override;

        bool CanEncodeTexture() override;
        bool Init(const EncoderConfig& config) override;
        void Encode(ID3D11Texture2D* tex2d, uint64_t frame_index, std::any extra) override;
        void Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, std::any extra) override;
        void Exit() override;

    private:
        std::shared_ptr<NVENCVideoEncoder> video_encoder_ = nullptr;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

#endif //GAMMARAY_UDP_PLUGIN_H

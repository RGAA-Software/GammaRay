//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_NVENC_ENCODER_PLUGIN_H
#define GAMMARAY_NVENC_ENCODER_PLUGIN_H

#include "plugin_interface/gr_video_encoder_plugin.h"

namespace tc
{

    class NVENCVideoEncoder;

    class NvencEncoderPlugin : public GrVideoEncoderPlugin {
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

        bool HasEncoderForMonitor(int8_t monitor_index) override;
        bool CanEncodeTexture() override;
        bool Init(const EncoderConfig& config, int8_t monitor_index) override;
        void Encode(ID3D11Texture2D* tex2d, uint64_t frame_index, const std::any& extra) override;
        void Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, const std::any& extra) override;
        void Exit(int8_t monitor_index) override;
        void ExitAll() override;

    private:
        std::map<int8_t, std::shared_ptr<NVENCVideoEncoder>> video_encoders_;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

#endif //GAMMARAY_UDP_PLUGIN_H

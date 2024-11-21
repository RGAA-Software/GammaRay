//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_MEDIA_RECORDER_PLUGIN_H
#define GAMMARAY_MEDIA_RECORDER_PLUGIN_H

#include "plugin_interface/gr_encoder_plugin.h"
#include "ffmpeg_encoder_defs.h"
extern "C" {
    #include "libavcodec/avcodec.h"
}

namespace tc
{

    class Data;
    class Image;

    class FFmpegEncoderPlugin : public GrEncoderPlugin {
    public:

        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        std::string GetPluginDescription() override;

        void On1Second() override;

        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        void InsertIdr() override;
        bool IsWorking() override;

        bool CanEncodeTexture() override;
        bool Init(const EncoderConfig& config) override;
        void Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, std::any extra) override;
        void Exit() override;

    private:
        AVCodecContext* context_ = nullptr;
        AVFrame* frame_ = nullptr;
        AVPacket* packet_ = nullptr;
        std::shared_ptr<Data> capture_data_ = nullptr;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

void* GetInstance() {
    static tc::FFmpegEncoderPlugin plugin;
    return (void*)&plugin;
}


#endif //GAMMARAY_UDP_PLUGIN_H

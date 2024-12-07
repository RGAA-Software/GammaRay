//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_MEDIA_RECORDER_PLUGIN_H
#define GAMMARAY_MEDIA_RECORDER_PLUGIN_H

#include "plugin_interface/gr_audio_encoder_plugin.h"

namespace tc
{

    class Data;
    class OpusAudioEncoder;
    class OpusAudioDecoder;

    class OpusEncoderPlugin : public GrAudioEncoderPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        void On1Second() override;

        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        void Encode(const std::shared_ptr<Data> &data, int sample, int channels, int bits) override;

    private:
        std::shared_ptr<OpusAudioEncoder> opus_encoder_ = nullptr;
        bool debug_opus_decoder_ = false;
        std::shared_ptr<OpusAudioDecoder> opus_decoder_ = nullptr;

        std::shared_ptr<Data> audio_cache_ = nullptr;
        int audio_callback_count_ = 0;

    };

}

extern "C" __declspec(dllexport) void* GetInstance();

void* GetInstance() {
    static tc::OpusEncoderPlugin plugin;
    return (void*)&plugin;
}


#endif //GAMMARAY_UDP_PLUGIN_H

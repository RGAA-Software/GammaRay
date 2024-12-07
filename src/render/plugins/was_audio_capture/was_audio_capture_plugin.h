//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_MEDIA_RECORDER_PLUGIN_H
#define GAMMARAY_MEDIA_RECORDER_PLUGIN_H

#include "plugin_interface/gr_data_provider_plugin.h"

namespace tc
{

    class IAudioCapture;

    class WasAudioCapturePlugin : public GrDataProviderPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        void On1Second() override;

        bool OnCreate(const tc::GrPluginParam &param) override;
        void OnCommand(const std::string &command) override;
        void StartProviding() override;
        void StopProviding() override;

    private:
        std::string audio_device_id_;
        int samples_ = 0;
        int channels_ = 0;
        int bits_ = 0;
        std::shared_ptr<IAudioCapture> audio_capture_ = nullptr;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

void* GetInstance() {
    static tc::WasAudioCapturePlugin plugin;
    return (void*)&plugin;
}


#endif //GAMMARAY_UDP_PLUGIN_H

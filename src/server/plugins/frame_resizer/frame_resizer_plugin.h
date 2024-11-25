//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_MEDIA_RECORDER_PLUGIN_H
#define GAMMARAY_MEDIA_RECORDER_PLUGIN_H

#include "plugin_interface/gr_frame_processor_plugin.h"

namespace tc
{
    class FrameRender;

    class FrameResizerPlugin : public GrFrameProcessorPlugin {
    public:
        FrameResizerPlugin();
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        void On1Second() override;
        ComPtr<ID3D11Texture2D> Process(ID3D11Texture2D* input, int target_width, int target_height) override;

    private:
        std::shared_ptr<FrameRender> frame_render_ = nullptr;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

#endif //GAMMARAY_UDP_PLUGIN_H

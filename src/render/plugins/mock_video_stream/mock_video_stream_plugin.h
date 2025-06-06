//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_MEDIA_RECORDER_PLUGIN_H
#define GAMMARAY_MEDIA_RECORDER_PLUGIN_H

#include "plugin_interface/gr_data_provider_plugin.h"
#include <opencv2/opencv.hpp>

namespace tc
{

    class MockVideoStreamPlugin : public GrDataProviderPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        std::string GetPluginDescription() override;
        bool OnCreate(const tc::GrPluginParam& param) override;
        void On1Second() override;
        void StartProviding() override;
        void StopProviding() override;

    private:
        void ReGenerate();

    private:
        cv::Mat mock_image_;
        int width_ = 640;
        int height_ = 480;
        uint64_t frame_index_ = 0;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

void* GetInstance() {
    static tc::MockVideoStreamPlugin plugin;
    return (void*)&plugin;
}


#endif //GAMMARAY_UDP_PLUGIN_H

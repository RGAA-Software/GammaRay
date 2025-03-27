//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_MEDIA_RECORDER_PLUGIN_H
#define GAMMARAY_MEDIA_RECORDER_PLUGIN_H

#include "plugin_interface/gr_stream_plugin.h"
#include <map>

namespace tc
{

    class ObjDetectorPlugin : public GrStreamPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;

        bool OnCreate(const tc::GrPluginParam& param) override;
        void On1Second() override;
        void OnRawVideoFrameRgba(const std::string& name, const std::shared_ptr<Image>& image) override;
        void OnRawVideoFrameYuv(const std::string& name, const std::shared_ptr<Image>& image) override;

    private:
        std::map<std::string, QLabel*> previewers_;

    };

}

extern "C" __declspec(dllexport) void* GetInstance();

void* GetInstance() {
    static tc::ObjDetectorPlugin plugin;
    return (void*)&plugin;
}


#endif //GAMMARAY_UDP_PLUGIN_H

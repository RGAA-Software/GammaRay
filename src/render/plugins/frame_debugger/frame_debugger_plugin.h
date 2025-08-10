//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_FRAME_DEBUGGER_PLUGIN_H
#define GAMMARAY_FRAME_DEBUGGER_PLUGIN_H

#include "plugin_interface/gr_stream_plugin.h"

namespace tc
{

    class File;

    class FrameDebuggerPlugin : public GrStreamPlugin {
    public:

        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        std::string GetPluginDescription() override;
        void On1Second() override;
        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;

        void OnVideoEncoderCreated(const GrPluginEncodedVideoType& type, int width, int height) override;
        // data: encode video frame, h264/h265/...
        void OnEncodedVideoFrame(const std::string& mon_name,
                                 const GrPluginEncodedVideoType& video_type,
                                 const std::shared_ptr<Data>& data,
                                 uint64_t frame_index,
                                 int frame_width,
                                 int frame_height,
                                 bool key) override;

    private:
        GrPluginEncodedVideoType encoded_video_type_{};
        bool save_encoded_video_ = false;
        std::shared_ptr<File> encoded_video_file_ = nullptr;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

void* GetInstance() {
    static tc::FrameDebuggerPlugin plugin;
    return (void*)&plugin;
}


#endif //GAMMARAY_UDP_PLUGIN_H

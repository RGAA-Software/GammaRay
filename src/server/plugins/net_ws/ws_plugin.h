//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_WS_PLUGIN_H
#define GAMMARAY_WS_PLUGIN_H

#include "plugin_interface/gr_plugin_interface.h"

namespace tc
{

    class WsPluginServer;

    class WsPlugin : public GrPluginInterface {
    public:

        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;

        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        void On1Second() override;
        void OnVideoEncoderCreated(const tc::GrPluginEncodedVideoType& type, int width, int height) override;
        void OnEncodedVideoFrame(const tc::GrPluginEncodedVideoType& video_type,
                                 const std::shared_ptr<Data> &data,
                                 uint64_t frame_index,
                                 int frame_width,
                                 int frame_height,
                                 bool key,
                                 int mon_idx,
                                 const std::string &display_name,
                                 int mon_left,
                                 int mon_top,
                                 int mon_right,
                                 int mon_bottom) override;
        void OnEncodedVideoFrameInProtobufFormat(const std::string& msg) override;

    private:

        std::shared_ptr<WsPluginServer> ws_server_ = nullptr;

    };

}

extern "C" __declspec(dllexport) void* GetInstance();

void* GetInstance() {
    static tc::WsPlugin plugin;
    return (void*)&plugin;
}


#endif //GAMMARAY_UDP_PLUGIN_H

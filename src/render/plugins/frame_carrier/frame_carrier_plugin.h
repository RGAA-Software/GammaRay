//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_FRAME_CARRIER_PLUGIN_H
#define GAMMARAY_FRAME_CARRIER_PLUGIN_H

#include "plugin_interface/gr_frame_carrier_plugin.h"

namespace tc
{

    class File;
    class VideoFrameCarrier;
    class GrFrameProcessorPlugin;

    class FrameCarrierPlugin : public GrFrameCarrierPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        std::string GetPluginDescription() override;
        void On1Second() override;
        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;

        // init carrier
        bool InitFrameCarrier(const tc::GrCarrierParams &params) override;

        // get frame resize plugin
        GrFrameProcessorPlugin* GetFrameResizePlugin(const std::string& mon_name);

        // copy texture
        std::shared_ptr<GrCarriedFrame> CopyTexture(const std::string& mon_name, uint64_t handle, uint64_t frame_index) override;

        // Map Texture from GPU -> CPU
        bool MapRawTexture(const std::string& mon_name, ID3D11Texture2D* texture, DXGI_FORMAT format, int height,
                                   std::function<void(const std::shared_ptr<Image>&)>&& rgba_cbk,
                                   std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk) override;

        // RGBA -> YUV
        bool ConvertRawImage(const std::string& mon_name, const std::shared_ptr<Image> image,
                                   std::function<void(const std::shared_ptr<Image>&)>&& rgba_cbk,
                                   std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk) override;
        // Resize information
        std::optional<GrFrameResizeInfo> GetFrameResizeInfo(const std::string &mon_name) override;

    private:
        std::shared_ptr<VideoFrameCarrier> GetFrameCarrier(const std::string& monitor_name);

    private:
        std::map<std::string, std::shared_ptr<VideoFrameCarrier>> frame_carriers_;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();


#endif //GAMMARAY_UDP_PLUGIN_H

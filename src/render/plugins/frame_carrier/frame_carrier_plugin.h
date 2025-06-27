//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_FRAME_CARRIER_PLUGIN_H
#define GAMMARAY_FRAME_CARRIER_PLUGIN_H

#include "plugin_interface/gr_frame_carrier_plugin.h"

namespace tc
{

    class File;

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

        //
        std::shared_ptr<GrCarriedFrame> CopyTexture(const std::string& mon_name, uint64_t handle, uint64_t frame_index) override;

    private:

    };

}

extern "C" __declspec(dllexport) void* GetInstance();


#endif //GAMMARAY_UDP_PLUGIN_H

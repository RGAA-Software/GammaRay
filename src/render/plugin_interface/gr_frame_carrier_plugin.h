//
// Created by RGAA on 27/06/2025.
//

#ifndef GAMMARAY_GR_FRAME_CARRIER_PLUGIN_H
#define GAMMARAY_GR_FRAME_CARRIER_PLUGIN_H

#include <memory>
#include "gr_plugin_interface.h"

namespace tc
{

    // Params
    class GrCarrierParams {
    public:
        std::string mon_name_;
        bool frame_resize_ = false;
    };

    // After copied
    class GrCarriedFrame {
    public:
#ifdef WIN32
        ID3D11Texture2D* texture_ = nullptr;
#endif
        uint64_t frame_index_ = 0;
    };

    class GrFrameCarrierPlugin : public GrPluginInterface {
    public:
        //
        virtual bool InitFrameCarrier(const GrCarrierParams& params) {
            carrier_params_ = params;
            return false;
        }

        // DDA / GDI / Hook -> Handle -> OpenShared -> Copy it
        virtual std::shared_ptr<GrCarriedFrame> CopyTexture(const std::string& mon_name, uint64_t handle, uint64_t frame_index) {
            return nullptr;
        }

    protected:
        GrCarrierParams carrier_params_;
    };

}

#endif //GAMMARAY_GR_FRAME_CARRIER_PLUGIN_H

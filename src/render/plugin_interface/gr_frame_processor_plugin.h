//
// Created by RGAA on 25/11/2024.
//

#ifndef GAMMARAY_GR_FRAME_PROCESSOR_H
#define GAMMARAY_GR_FRAME_PROCESSOR_H

#include "gr_plugin_interface.h"
#include <d3d11.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

namespace tc
{

    enum class FrameProcessorType {
        kPrev, // before encode
        kPost, // maybe for debugging
    };

    class GrFrameProcessorPlugin : public GrPluginInterface {
    public:
        GrFrameProcessorPlugin();
        virtual ComPtr<ID3D11Texture2D> Process(ID3D11Texture2D* input, uint64_t adapter_uid, const std::string& monitor_name, int target_width, int target_height) = 0;

    public:
        int priority_ = 0;
        FrameProcessorType processor_type_;
    };

}

#endif //GAMMARAY_GR_FRAME_PROCESSOR_H

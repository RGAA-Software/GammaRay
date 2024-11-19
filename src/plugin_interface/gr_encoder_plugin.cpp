//
// Created by hy on 19/11/2024.
//

#include "gr_encoder_plugin.h"

namespace tc
{

    GrEncoderPlugin::GrEncoderPlugin() : GrPluginInterface() {
        plugin_type_ = GrPluginType::kEncoder;
    }

    GrEncoderPlugin::~GrEncoderPlugin() {

    }

    bool GrEncoderPlugin::OnCreate(const tc::GrPluginParam &param) {
        return true;
    }

    bool GrEncoderPlugin::OnDestroy() {
        return true;
    }

    void GrEncoderPlugin::Encode(uint64_t handle, uint64_t frame_index) {

    }

    void GrEncoderPlugin::Encode(ID3D11Texture2D* tex2d) {

    }

    void GrEncoderPlugin::Encode(const std::shared_ptr<Image>& i420_data, uint64_t frame_index) {

    }

    void GrEncoderPlugin::InsertIdr() {

    }

}

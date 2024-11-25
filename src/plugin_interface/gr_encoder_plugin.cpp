//
// Created by RGAA on 19/11/2024.
//

#include "gr_encoder_plugin.h"
#include "tc_common_new/log.h"

namespace tc
{

    GrEncoderPlugin::GrEncoderPlugin() : GrPluginInterface() {
        plugin_type_ = GrPluginType::kEncoder;
    }

    GrEncoderPlugin::~GrEncoderPlugin() {

    }

    bool GrEncoderPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrPluginInterface::OnCreate(param);
        return true;
    }

    bool GrEncoderPlugin::OnDestroy() {
        return true;
    }

    bool GrEncoderPlugin::CanEncodeTexture() {
        return false;
    }

    bool GrEncoderPlugin::Init(const EncoderConfig& config) {
        LOGI("GrEncoderPlugin Init, {}x{}", config.encode_width, config.encode_height);
        encoder_config_ = config;
        input_frame_width_ = config.width;
        input_frame_height_ = config.height;
        out_width_ = config.encode_width;
        out_height_ = config.encode_height;
        refresh_rate_ = config.fps;
        d3d11_device_ = config.d3d11_device_;
        d3d11_device_context_ = config.d3d11_device_context_;
        return true;
    }

    void GrEncoderPlugin::Encode(ID3D11Texture2D* tex2d, uint64_t frame_index, std::any extra) {

    }

    void GrEncoderPlugin::Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, std::any extra) {

    }

    void GrEncoderPlugin::InsertIdr() {
        insert_idr_ = true;
    }

    EncoderConfig GrEncoderPlugin::GetEncoderConfig() {
        return encoder_config_;
    }

    void GrEncoderPlugin::Exit() {

    }

}

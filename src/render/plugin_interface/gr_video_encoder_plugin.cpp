//
// Created by RGAA on 19/11/2024.
//

#include "gr_video_encoder_plugin.h"
#include "tc_common_new/log.h"

namespace tc
{

    GrVideoEncoderPlugin::GrVideoEncoderPlugin() : GrPluginInterface() {
        plugin_type_ = GrPluginType::kEncoder;
    }

    GrVideoEncoderPlugin::~GrVideoEncoderPlugin() {

    }

    bool GrVideoEncoderPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrPluginInterface::OnCreate(param);
        return true;
    }

    bool GrVideoEncoderPlugin::OnDestroy() {
        return true;
    }

    bool GrVideoEncoderPlugin::CanEncodeTexture() {
        return false;
    }

    bool GrVideoEncoderPlugin::Init(const EncoderConfig& config, const std::string& monitor_name) {
        LOGI("GrVideoEncoderPlugin Init, {}x{}", config.encode_width, config.encode_height);
        encoder_configs_[monitor_name] = config;
        out_width_ = config.encode_width;
        out_height_ = config.encode_height;
        refresh_rate_ = config.fps;
        //d3d11_device_ = config.d3d11_device_;
        //d3d11_device_context_ = config.d3d11_device_context_;
        return true;
    }

    void GrVideoEncoderPlugin::Encode(ID3D11Texture2D* tex2d, uint64_t frame_index, const std::any& extra) {

    }

    void GrVideoEncoderPlugin::Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, const std::any& extra) {

    }

    void GrVideoEncoderPlugin::InsertIdr() {
        insert_idr_ = true;
    }

    std::optional<EncoderConfig> GrVideoEncoderPlugin::GetEncoderConfig(const std::string& monitor_name) {

        if (encoder_configs_.find(monitor_name) != encoder_configs_.end()) {
            return encoder_configs_[monitor_name];
        }
        return std::nullopt;
    }

    void GrVideoEncoderPlugin::Exit(const std::string& monitor_name) {

    }

    void GrVideoEncoderPlugin::ExitAll() {

    }

}

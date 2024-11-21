//
// Created RGAA on 15/11/2024.
//

#include "nvenc_encoder_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "nvenc_encoder_defs.h"

static void* GetInstance() {
    static tc::NvencEncoderPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{
    std::string NvencEncoderPlugin::GetPluginName() {
        return kNvencPluginName;
    }

    std::string NvencEncoderPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t NvencEncoderPlugin::GetVersionCode() {
        return 110;
    }

    void NvencEncoderPlugin::On1Second() {

    }

    bool NvencEncoderPlugin::OnCreate(const tc::GrPluginParam& param) {
        GrEncoderPlugin::OnCreate(param);
        return true;
    }

    bool NvencEncoderPlugin::OnDestroy() {
        GrEncoderPlugin::OnDestroy();
        return true;
    }

    void NvencEncoderPlugin::InsertIdr() {
        GrEncoderPlugin::InsertIdr();
    }

    bool NvencEncoderPlugin::CanEncodeTexture() {
        return true;
    }

    bool NvencEncoderPlugin::Init(const EncoderConfig& config) {

        return false;
    }

    void NvencEncoderPlugin::Encode(ID3D11Texture2D* tex2d, uint64_t frame_index, std::any extra) {

    }

    void NvencEncoderPlugin::Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, std::any extra) {

    }

    void NvencEncoderPlugin::Exit() {

    }

}

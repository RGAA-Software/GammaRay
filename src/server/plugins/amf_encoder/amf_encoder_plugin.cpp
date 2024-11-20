//
// Created RGAA on 15/11/2024.
//

#include "amf_encoder_plugin.h"
#include "plugin_interface/gr_plugin_events.h"

namespace tc
{

    std::string AmfEncoderPlugin::GetPluginName() {
        return "Media Recorder Plugin";
    }

    std::string AmfEncoderPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t AmfEncoderPlugin::GetVersionCode() {
        return 110;
    }

    bool AmfEncoderPlugin::CanEncodeTexture() {
        return true;
    }

    void AmfEncoderPlugin::On1Second() {

    }

    bool AmfEncoderPlugin::OnCreate(const tc::GrPluginParam &param) {
        return true;
    }

    bool AmfEncoderPlugin::OnDestroy() {
        return true;
    }

    void AmfEncoderPlugin::InsertIdr() {
        GrEncoderPlugin::InsertIdr();
    }

    bool AmfEncoderPlugin::IsWorking() {
        return init_success_ && plugin_enabled_;
    }

    bool AmfEncoderPlugin::Init(const EncoderConfig& config) {
        return true;
    }

    void AmfEncoderPlugin::Encode(ID3D11Texture2D* tex2d, uint64_t frame_index) {

    }

    void AmfEncoderPlugin::Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index) {

    }

    void AmfEncoderPlugin::Exit() {

    }

}

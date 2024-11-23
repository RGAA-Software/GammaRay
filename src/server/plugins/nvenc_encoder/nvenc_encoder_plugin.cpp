//
// Created RGAA on 15/11/2024.
//

#include "nvenc_encoder_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "nvenc_encoder_defs.h"
#include "nvenc_video_encoder.h"
#include "tc_common_new/log.h"
#include "server/plugins/plugin_ids.h"

static void* GetInstance() {
    static tc::NvencEncoderPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{
    std::string NvencEncoderPlugin::GetPluginId() {
        return kNvencEncoderPluginId;
    }

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
        if (IsWorking()) {
            video_encoder_->InsertIdr();
        }
    }

    bool NvencEncoderPlugin::IsWorking() {
        return init_success_ && plugin_enabled_ && video_encoder_;
    }

    bool NvencEncoderPlugin::CanEncodeTexture() {
        return true;
    }

    bool NvencEncoderPlugin::Init(const EncoderConfig& config) {
        if (video_encoder_) {
            video_encoder_->Exit();
        }
        video_encoder_ = std::make_shared<NVENCVideoEncoder>(this, config.adapter_uid_);
        LOGI("config bitrate: {}", config.bitrate);
        init_success_ = video_encoder_->Initialize(config);
        if (!init_success_) {
            LOGE("Init NVENC encoder failed!");
        }
        return init_success_;
    }

    void NvencEncoderPlugin::Encode(ID3D11Texture2D* tex2d, uint64_t frame_index, std::any extra) {
        if (IsWorking()) {
            video_encoder_->Encode(tex2d, frame_index, extra);
        }
    }

    void NvencEncoderPlugin::Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, std::any extra) {

    }

    void NvencEncoderPlugin::Exit() {

    }

}

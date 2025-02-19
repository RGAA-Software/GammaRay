//
// Created RGAA on 15/11/2024.
//

#include "nvenc_encoder_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "nvenc_encoder_defs.h"
#include "nvenc_video_encoder.h"
#include "tc_common_new/log.h"
#include "render/plugins/plugin_ids.h"

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
        GrVideoEncoderPlugin::OnCreate(param);
        return true;
    }

    bool NvencEncoderPlugin::OnDestroy() {
        GrVideoEncoderPlugin::OnDestroy();
        return true;
    }

    void NvencEncoderPlugin::InsertIdr() {
        GrVideoEncoderPlugin::InsertIdr();
        if (IsWorking()) {
            for (const auto& [monitor_index, video_encoder] : video_encoders_) {
                video_encoder->InsertIdr();
            }
        }
    }

    bool NvencEncoderPlugin::IsWorking() {
        return init_success_ && plugin_enabled_ && !video_encoders_.empty();
    }

    bool NvencEncoderPlugin::CanEncodeTexture() {
        return true;
    }

    bool NvencEncoderPlugin::HasEncoderForMonitor(const std::string& monitor_name) {
        return video_encoders_.find(monitor_name) != video_encoders_.end();
    }

    bool NvencEncoderPlugin::Init(const EncoderConfig& config, const std::string& monitor_name) {
        video_encoders_[monitor_name] = std::make_shared<NVENCVideoEncoder>(this, config.adapter_uid_);
        LOGI("config bitrate: {} for monitor: {}", config.bitrate, monitor_name);
        init_success_ = video_encoders_[monitor_name]->Initialize(config);
        if (!init_success_) {
            LOGE("Init NVENC encoder failed!");
        }
        return init_success_;
    }

    void NvencEncoderPlugin::Encode(ID3D11Texture2D* tex2d, uint64_t frame_index, const std::any& extra) {
        auto cap_video_msg = std::any_cast<CaptureVideoFrame>(extra);
        auto monitor_name = std::string(cap_video_msg.display_name_);
        if (IsWorking() && HasEncoderForMonitor(monitor_name)) {
            video_encoders_[monitor_name]->Encode(tex2d, frame_index, extra);
        }
    }

    void NvencEncoderPlugin::Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, const std::any& extra) {

    }

    void NvencEncoderPlugin::Exit(const std::string& monitor_name) {
        if (video_encoders_.find(monitor_name) != video_encoders_.end()) {
            video_encoders_[monitor_name]->Exit();
            video_encoders_.erase(monitor_name);
        }
    }

    void NvencEncoderPlugin::ExitAll() {

    }

}

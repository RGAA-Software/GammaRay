//
// Created RGAA on 15/11/2024.
//

#include "amf_encoder_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "video_encoder_vce.h"
#include "amf_encoder_defs.h"
#include "tc_common_new/log.h"
#include "render/plugins/plugin_ids.h"

static void* GetInstance() {
    static tc::AmfEncoderPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    std::string AmfEncoderPlugin::GetPluginId() {
        return kAmfEncoderPluginId;
    }

    std::string AmfEncoderPlugin::GetPluginName() {
        return kAmfPluginName;
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

    bool AmfEncoderPlugin::OnCreate(const tc::GrPluginParam& param) {
        tc::GrVideoEncoderPlugin::OnCreate(param);
        return true;
    }

    bool AmfEncoderPlugin::OnDestroy() {
        return true;
    }

    void AmfEncoderPlugin::InsertIdr() {
        GrVideoEncoderPlugin::InsertIdr();
        if (IsWorking()) {
            for (const auto& [monitor_index, video_encoder] : video_encoders_) {
                video_encoder->InsertIdr();
            }
        }
    }

    bool AmfEncoderPlugin::HasEncoderForMonitor(int8_t monitor_index) {
        return video_encoders_.find(monitor_index) == video_encoders_.end();
    }

    bool AmfEncoderPlugin::IsWorking() {
        return init_success_ && plugin_enabled_ && !video_encoders_.empty();
    }

    bool AmfEncoderPlugin::Init(const EncoderConfig& config, int8_t monitor_index) {
        GrVideoEncoderPlugin::Init(config, monitor_index);
        video_encoders_[monitor_index] = std::make_shared<VideoEncoderVCE>(this, config.adapter_uid_);
        init_success_ = video_encoders_[monitor_index]->Initialize(config);
        if (!init_success_) {
            LOGE("AMF encoder init failed!");
            return false;
        }
        LOGI("Video encoder init success for monitor: {}", monitor_index);
        return init_success_;
    }

    void AmfEncoderPlugin::Encode(ID3D11Texture2D* tex2d, uint64_t frame_index, const std::any& extra) {
        auto cap_video_msg = std::any_cast<CaptureVideoFrame>(extra);
        if (IsWorking()) {
            if (video_encoders_.find(cap_video_msg.monitor_index_) == video_encoders_.end()) {
                LOGE("Not found video encoder for monitor: {}", cap_video_msg.monitor_index_);
                return;
            }
            auto video_encoder = video_encoders_[cap_video_msg.monitor_index_];
            video_encoder->Encode(tex2d, frame_index, extra);
        }
        else {
            LOGI("Amf encoder is not working, ignore it.");
        }
    }

    void AmfEncoderPlugin::Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, const std::any& extra) {

    }

    void AmfEncoderPlugin::Exit(int8_t monitor_index) {
        if (video_encoders_.find(monitor_index) != video_encoders_.end()) {
            video_encoders_[monitor_index]->Exit();
            video_encoders_.erase(monitor_index);
        }
    }

    void AmfEncoderPlugin::ExitAll() {
        for (const auto& [monitor_index, video_encoder] : video_encoders_) {
            if (video_encoder) {
                video_encoder->Exit();
            }
        }
        video_encoders_.clear();
        LOGI("Amf encoders all exit.");
    }

}

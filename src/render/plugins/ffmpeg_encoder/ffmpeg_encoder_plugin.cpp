//
// Created RGAA on 15/11/2024.
//

#include "ffmpeg_encoder_plugin.h"
#include "plugin_interface/gr_plugin_events.h"

#include <libyuv.h>

#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/image.h"
#include "tc_common_new/win32/d3d_debug_helper.h"
#include "tc_common_new/file.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/defer.h"
#include "render/plugins/plugin_ids.h"
#include "ffmpeg_encoder.h"
#include <Winerror.h>

void* GetInstance() {
    static tc::FFmpegEncoderPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    std::string FFmpegEncoderPlugin::GetPluginId() {
        return kFFmpegEncoderPluginId;
    }

    std::string FFmpegEncoderPlugin::GetPluginName() {
        return kFFmpegPluginName;
    }

    std::string FFmpegEncoderPlugin::GetVersionName() {
        return plugin_version_name_;
    }

    uint32_t FFmpegEncoderPlugin::GetVersionCode() {
        return plugin_version_code_;
    }

    std::string FFmpegEncoderPlugin::GetPluginDescription() {
        return plugin_desc_;
    }

    void FFmpegEncoderPlugin::On1Second() {

    }

    bool FFmpegEncoderPlugin::OnCreate(const tc::GrPluginParam& plugin_param) {
        tc::GrVideoEncoderPlugin::OnCreate(plugin_param);

        return true;
    }

    bool FFmpegEncoderPlugin::OnDestroy() {
        return true;
    }

    void FFmpegEncoderPlugin::InsertIdr() {
        GrVideoEncoderPlugin::InsertIdr();
        for (const auto& [monitor_index, video_encoder] : video_encoders_) {
            video_encoder->InsertIdr();
        }
    }

    bool FFmpegEncoderPlugin::IsWorking() {
        return !video_encoders_.empty() && plugin_enabled_;
    }

    bool FFmpegEncoderPlugin::CanEncodeTexture() {
        return false;
    }

    bool FFmpegEncoderPlugin::HasEncoderForMonitor(const std::string& monitor_name) {
        return video_encoders_.find(monitor_name) != video_encoders_.end();
    }

    bool FFmpegEncoderPlugin::Init(const EncoderConfig& config, const std::string& monitor_name) {
        GrVideoEncoderPlugin::Init(config, monitor_name);
        auto encoder = std::make_shared<FFmpegEncoder>(this);
        auto ok = encoder->Init(config, monitor_name);
        if (!ok) {
            LOGE("Init ffmpeg encoder for: {} failed.", monitor_name);
            return false;
        }
        LOGI("FFmpeg encoder init success for: {}", monitor_name);
        video_encoders_[monitor_name] = encoder;
        return true;
    }

    void FFmpegEncoderPlugin::Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, const std::any& extra) {
        auto cap_video_frame = std::any_cast<CaptureVideoFrame>(extra);
        auto monitor_name = std::string(cap_video_frame.display_name_);
        if (!HasEncoderForMonitor(monitor_name)) {
            return;
        }
        video_encoders_[monitor_name]->Encode(i420_image, frame_index, extra);
    }

    void FFmpegEncoderPlugin::Exit(const std::string& monitor_name) {
        if (HasEncoderForMonitor(monitor_name)) {
            video_encoders_[monitor_name]->Exit();
            video_encoders_.erase(monitor_name);
        }
    }

    void FFmpegEncoderPlugin::ExitAll() {
        for (const auto& [monitor, video_encoder] : video_encoders_) {
            video_encoder->Exit();
        }
        video_encoders_.clear();
    }

    std::map<std::string, WorkingEncoderInfoPtr> FFmpegEncoderPlugin::GetWorkingCapturesInfo() {
        std::map<std::string, WorkingEncoderInfoPtr> result;
        for (const auto& [monitor, video_encoder] : video_encoders_) {
            result.insert({monitor, std::make_shared<WorkingEncoderInfo>(WorkingEncoderInfo {
                .target_name_ = monitor,
                .fps_ = video_encoder->GetEncodeFps(),
                .encoder_name_ = "S/W",
            })});
        }
        return result;
    }

}

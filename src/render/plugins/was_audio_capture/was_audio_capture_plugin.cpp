//
// Created RGAA on 15/11/2024.
//

#include "was_audio_capture_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "render/plugins/plugin_ids.h"
#include "audio_capture.h"
#include "wasapi_audio_capture.h"

namespace tc
{
    std::string WasAudioCapturePlugin::GetPluginId() {
        return kWasAudioCapturePluginId;
    }

    std::string WasAudioCapturePlugin::GetPluginName() {
        return "Was Audio Capture Plugin";
    }

    std::string WasAudioCapturePlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t WasAudioCapturePlugin::GetVersionCode() {
        return 110;
    }

    void WasAudioCapturePlugin::On1Second() {

    }

    bool WasAudioCapturePlugin::OnCreate(const tc::GrPluginParam& param) {
        GrDataProviderPlugin::OnCreate(param);
        auto key_audio_device_id = "audio_device_id";
        if (HasParam(key_audio_device_id)) {
            audio_device_id_ = GetParam<std::string>(key_audio_device_id);
        }

        return true;
    }

    void WasAudioCapturePlugin::OnCommand(const std::string& command) {

    }

    void WasAudioCapturePlugin::StartProviding() {
        audio_capture_ = WASAPIAudioCapture::Make(audio_device_id_);
        audio_capture_->RegisterFormatCallback([=, this](int samples, int channels, int bits) {
            this->samples_ = samples;
            this->channels_ = channels;
            this->bits_ = bits;
        });

        audio_capture_->RegisterDataCallback([=, this](const std::shared_ptr<Data>& data) {
            auto event = std::make_shared<GrPluginRawAudioFrameEvent>();
            event->full_data_ = data;
            event->sample_rate_ = this->samples_;
            event->channels_ = this->channels_;
            event->bits_ = this->bits_;
            CallbackEvent(event);
        });

        audio_capture_->RegisterSplitDataCallback([=, this](const auto& left, const auto& right) {
            auto event = std::make_shared<GrPluginSplitRawAudioFrameEvent>();
            event->left_ch_data_ = left;
            event->right_ch_data_ = right;
            event->sample_rate_ = this->samples_;
            event->channels_ = this->channels_;
            event->bits_ = this->bits_;
            CallbackEvent(event);
        });

        audio_capture_->Start();
    }

    void WasAudioCapturePlugin::StopProviding() {
        if (audio_capture_) {
            audio_capture_->Pause();
            audio_capture_->Stop();
        }
    }

}

//
// Created by RGAA on 7/12/2024.
//

#include "gr_audio_encoder_plugin.h"

namespace tc
{

    GrAudioEncoderPlugin::GrAudioEncoderPlugin() {

    }

    GrAudioEncoderPlugin::~GrAudioEncoderPlugin() {

    }

    bool GrAudioEncoderPlugin::OnCreate(const tc::GrPluginParam& param) {
        GrPluginInterface::OnCreate(param);

        return true;
    }

    bool GrAudioEncoderPlugin::OnDestroy() {

        return true;
    }

    void GrAudioEncoderPlugin::Encode(const std::shared_ptr<Data>& data, int sample, int channels, int bits) {

    }

}
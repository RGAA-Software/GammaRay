//
// Created RGAA on 15/11/2024.
//

#include "opus_encoder_plugin.h"
#include "render/plugins/plugin_ids.h"
#include "tc_opus_codec_new/opus_codec.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/file.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/memory_stat.h"
#include "plugin_interface/gr_plugin_events.h"
#include "plugin_interface/gr_plugin_context.h"

namespace tc
{
    std::string OpusEncoderPlugin::GetPluginId() {
        return kOpusEncoderPluginId;
    }

    std::string OpusEncoderPlugin::GetPluginName() {
        return "OPUS Encoder";
    }

    std::string OpusEncoderPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t OpusEncoderPlugin::GetVersionCode() {
        return 110;
    }

    std::string OpusEncoderPlugin::GetPluginDescription() {
        return "OPUS audio encoder";
    }

    void OpusEncoderPlugin::On1Second() {
#if MEMORY_STST_ON
        plugin_context_->PostWorkTask([=, this]() {
            auto info = MemoryStat::Instance()->GetStatInfo();
            LOGI("Memory usage: {}", info.Dump());
        });
#endif
    }

    bool OpusEncoderPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrAudioEncoderPlugin::OnCreate(param);
        auto key_save_debug_file = "save_debug_file";
        if (HasParam(key_save_debug_file)) {
            debug_opus_decoder_ = GetConfigParam<bool>(key_save_debug_file);
        }

        return true;
    }

    bool OpusEncoderPlugin::OnDestroy() {
        GrAudioEncoderPlugin::OnDestroy();
        return true;
    }

    void OpusEncoderPlugin::Encode(const std::shared_ptr<Data> &data, int samples, int channels, int bits) {
        if (!opus_encoder_) {
            // audio cache
            audio_cache_ = Data::Make(nullptr, 1024*16);
            LOGI("audio format, samples: {}, channels: {}, bits: {}", samples, channels, bits);
            opus_encoder_ = std::make_shared<OpusAudioEncoder>(samples, channels, bits, OPUS_APPLICATION_AUDIO);
            if (!opus_encoder_->valid()) {
                opus_encoder_ = nullptr;
                return;
            }
            opus_encoder_->SetComplexity(8);
        }

        if (debug_opus_decoder_) {
            static auto pcm_file = File::OpenForWriteB("1.opus.encoder.plugin.origin.pcm");
            pcm_file->Append((char*)data->DataAddr(), data->Size());
        }

        PostWorkTask([=, this]() {
            audio_cache_->Append(data->DataAddr(), data->Size());
            // 2 or 6
            if (++audio_callback_count_ < 2) {
                return;
            }

            //int frame_size = data->Size() / 2 / 2;
            int frame_size = audio_cache_->Offset()/2/2;
            auto encoded_frames = opus_encoder_->Encode(audio_cache_->CStr(), audio_cache_->Offset(), frame_size);
            for (const auto& ef : encoded_frames) {
                auto encoded_data = Data::Make((char*)ef.data(), ef.size());

                auto event = std::make_shared<GrPluginEncodedAudioFrameEvent>();
                event->sample_rate_ = samples;
                event->channels_ = channels;
                event->bits_ = bits;
                event->frame_size_ = frame_size;
                event->data_ = encoded_data;
                CallbackEventDirectly(event);

                if (debug_opus_decoder_) {
                    if (!opus_decoder_) {
                        opus_decoder_ = std::make_shared<OpusAudioDecoder>(opus_encoder_->SampleRate(), opus_encoder_->Channels());
                    }
                    std::vector<unsigned char> buffer(ef.begin(), ef.end());
                    auto pcm_data = opus_decoder_->Decode(buffer, frame_size, false);
                    static auto pcm_file = File::OpenForWriteB("1.test.pcm");
                    pcm_file->Append((char*)pcm_data.data(), pcm_data.size()*2);
                }
            }

            audio_cache_->Reset();
            audio_callback_count_ = 0;

        });
    }

}

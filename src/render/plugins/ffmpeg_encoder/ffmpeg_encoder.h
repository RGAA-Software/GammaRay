//
// Created by RGAA on 18/02/2025.
//

#ifndef GAMMARAY_FFMPEG_ENCODER_H
#define GAMMARAY_FFMPEG_ENCODER_H

#include <any>
#include <memory>
#include "ffmpeg_encoder_defs.h"
#include "tc_encoder_new/encoder_config.h"
#include "tc_common_new/fps_stat.h"

extern "C" {
    #include "libavcodec/avcodec.h"
}

namespace tc
{

    class Data;
    class Image;
    class FFmpegEncoderPlugin;

    class FFmpegEncoder {
    public:
        explicit FFmpegEncoder(FFmpegEncoderPlugin* plugin);
        bool Init(const EncoderConfig& config, const std::string& monitor_name);
        void Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, const std::any& extra);
        void InsertIdr();
        void Exit();
        int32_t GetEncodeFps();

    private:
        FFmpegEncoderPlugin* plugin_ = nullptr;
        AVCodecContext* context_ = nullptr;
        AVFrame* frame_ = nullptr;
        AVPacket* packet_ = nullptr;
        int gop_size_ = 60;
        int bitrate_ = 10000000; // 10Mbps
        bool insert_idr_ = false;
        EncoderConfig encoder_config_;
        std::shared_ptr<FpsStat> fps_stat_ = nullptr;
    };

}

#endif //GAMMARAY_FFMPEG_ENCODER_H

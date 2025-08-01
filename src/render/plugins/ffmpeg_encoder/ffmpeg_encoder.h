//
// Created by RGAA on 18/02/2025.
//

#ifndef GAMMARAY_FFMPEG_ENCODER_H
#define GAMMARAY_FFMPEG_ENCODER_H

#include <any>
#include <memory>
#include <mutex>
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
        void Encode(const std::shared_ptr<Image>& image, uint64_t frame_index, const std::any& extra);
        void InsertIdr();
        void Exit();
        int32_t GetEncodeFps();
        std::vector<int32_t> GetEncodeDurations();
        std::string GetDisplayEncoderName();

    private:
        FFmpegEncoderPlugin* plugin_ = nullptr;
        AVCodecContext* codec_ctx_ = nullptr;
        AVFrame* frame_ = nullptr;
        AVPacket* packet_ = nullptr;
        int gop_size_ = 60;
        bool insert_idr_ = false;
        EncoderConfig encoder_config_;
        std::shared_ptr<FpsStat> fps_stat_ = nullptr;
        std::deque<int32_t> encode_durations_;
        std::once_flag init_log_flag_;
        std::string display_encoder_name_;
    private:
        void InitLog();
    };

}

#endif //GAMMARAY_FFMPEG_ENCODER_H

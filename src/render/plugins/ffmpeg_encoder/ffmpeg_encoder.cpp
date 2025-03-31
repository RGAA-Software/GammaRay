//
// Created by RGAA on 18/02/2025.
//

#include "ffmpeg_encoder.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/image.h"
#include "tc_common_new/file.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/defer.h"
#include "plugin_interface/gr_plugin_events.h"
#include "ffmpeg_encoder_plugin.h"

namespace tc
{

    FFmpegEncoder::FFmpegEncoder(FFmpegEncoderPlugin* plugin) {
        plugin_ = plugin;
    }

    bool FFmpegEncoder::Init(const EncoderConfig& config, const std::string& monitor_name) {
        encoder_config_ = config;
        auto encoder_id = config.codec_type == EVideoCodecType::kHEVC ? AV_CODEC_ID_HEVC : AV_CODEC_ID_H264;
        const AVCodec* encoder = avcodec_find_encoder(encoder_id);

        context_ = avcodec_alloc_context3(encoder);
        if (!context_) {
            LOGE("avcodec_alloc_context3 error!");
            return false;
        }

        // test
        if (encoder_config_.fps  <= 0) {
            encoder_config_.fps = 60;
        }

        context_->width = encoder_config_.encode_width;
        context_->height = encoder_config_.encode_height;
        context_->time_base = { 1, encoder_config_.fps };
        context_->framerate = { encoder_config_.fps, 1};
        context_->flags |= AV_CODEC_FLAG_LOW_DELAY;
        context_->pix_fmt = AV_PIX_FMT_YUV420P;
        context_->thread_count = std::min(16, (int)std::thread::hardware_concurrency());
        context_->thread_type = FF_THREAD_SLICE;
        context_->gop_size = gop_size_;
        context_->max_b_frames = 0;
        context_->bit_rate = bitrate_;

        LOGI("ffmpeg encoder config:");
        LOGI("bitrate: {}", context_->bit_rate);
        LOGI("format: {}", (config.codec_type == EVideoCodecType::kHEVC ? "HEVC" : "H264"));
        LOGI("refresh rate(fps): {}", config.fps);
        LOGI("thread count: {}", context_->thread_count);
        LOGI("gop size: {}", context_->gop_size);

        AVDictionary* param = nullptr;
        if(encoder_id == AV_CODEC_ID_H264) {
            //av_dict_set(&param, "preset", "superfast",   0);
            av_dict_set(&param, "preset", "ultrafast",   0);
            av_dict_set(&param, "tune",   "zerolatency", 0);
            av_dict_set(&param, "crf", "23", 0);
            av_dict_set(&param, "forced-idr", "1", 0);
        }
        if(encoder_id == AV_CODEC_ID_H265) {
            av_dict_set(&param, "x265-params", "qp=20", 0);
            av_dict_set(&param, "preset", "ultrafast", 0);
            av_dict_set(&param, "tune", "zero-latency", 0);
        }

        auto ret = avcodec_open2(context_, encoder, &param);
        if (ret != 0) {
            LOGE("avcodec_open2 error : {}", ret);
            return false;
        }

        frame_ = av_frame_alloc();
        frame_->width = context_->width;
        frame_->height = context_->height;
        frame_->format = context_->pix_fmt;

        av_frame_get_buffer(frame_, 0);
        packet_ = av_packet_alloc();
        LOGI("Line 1: {} 2: {} 3: {}", frame_->linesize[0], frame_->linesize[1], frame_->linesize[2]);
        return true;
    }

    void FFmpegEncoder::Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, const std::any& extra) {
        auto cap_video_frame = std::any_cast<CaptureVideoFrame>(extra);
        auto img_width = i420_image->width;
        auto img_height = i420_image->height;
        auto i420_data = i420_image->data;

        // re-create when width/height changed
        // todo

        frame_->pts = (int64_t)frame_index;

        if (insert_idr_) {
            insert_idr_ = false;
            frame_->key_frame = 1;
            frame_->pict_type = AV_PICTURE_TYPE_I;
            LOGI("Insert an I Frame!");
        } else {
            frame_->key_frame = 0;
            frame_->pict_type = AV_PICTURE_TYPE_NONE;
        }

        int y_size =  img_width * img_height;
        int uv_size = img_width * img_height / 4;
        memcpy(frame_->data[0], i420_data->CStr(), y_size);
        memcpy(frame_->data[1], i420_data->CStr() + y_size, uv_size);
        memcpy(frame_->data[2], i420_data->CStr() + y_size + uv_size, uv_size);

        int send_result = avcodec_send_frame(context_, frame_);
        while (send_result >= 0) {
            int receiveResult = avcodec_receive_packet(context_, packet_);
            if (receiveResult == AVERROR(EAGAIN) || receiveResult == AVERROR_EOF) {
                break;
            }

            bool key_frame = (packet_->flags & AV_PKT_FLAG_KEY);
            auto encoded_data = Data::Make((char*)packet_->data, packet_->size);

            auto event = std::make_shared<GrPluginEncodedVideoFrameEvent>();
            event->type_ = [=, this]() {
                if (encoder_config_.codec_type == EVideoCodecType::kHEVC) {
                    return GrPluginEncodedVideoType::kH265;
                } else if (encoder_config_.codec_type == EVideoCodecType::kH264) {
                    return GrPluginEncodedVideoType::kH264;
                } else {
                    return GrPluginEncodedVideoType::kH264;
                }
            }();
            event->data_ = encoded_data;
            event->frame_width_ = img_width;
            event->frame_height_ = img_height;
            event->key_frame_ = key_frame;
            event->frame_index_ = frame_index;
            event->extra_ = extra;
            plugin_->CallbackEvent(event);

            av_packet_unref(packet_);
        }
    }

    void FFmpegEncoder::InsertIdr() {
        insert_idr_ = true;
    }

    void FFmpegEncoder::Exit() {

    }

}
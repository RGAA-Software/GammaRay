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
#include "tc_common_new/string_ext.h"
#include "plugin_interface/gr_plugin_events.h"
#include "ffmpeg_encoder_plugin.h"

namespace tc
{

    FFmpegEncoder::FFmpegEncoder(FFmpegEncoderPlugin* plugin) {
        plugin_ = plugin;
        fps_stat_ = std::make_shared<FpsStat>();
    }

    bool FFmpegEncoder::Init(const EncoderConfig& config, const std::string& monitor_name) {
        InitLog();
        encoder_config_ = config;
        auto encoder_id = config.codec_type == EVideoCodecType::kHEVC ? AV_CODEC_ID_HEVC : AV_CODEC_ID_H264;
        const AVCodec* encoder = avcodec_find_encoder(encoder_id);

        codec_ctx_ = avcodec_alloc_context3(encoder);
        if (!codec_ctx_) {
            LOGE("avcodec_alloc_context3 error!");
            return false;
        }

        // test
        if (encoder_config_.fps  <= 0) {
            encoder_config_.fps = 60;
        }

        codec_ctx_->width = encoder_config_.encode_width;
        codec_ctx_->height = encoder_config_.encode_height;
        codec_ctx_->time_base = { 1, encoder_config_.fps };
        codec_ctx_->framerate = { encoder_config_.fps, 1};
        codec_ctx_->flags |= AV_CODEC_FLAG_LOW_DELAY;
        codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;
        if (encoder_config_.enable_full_color_mode_) {
            codec_ctx_->pix_fmt = AV_PIX_FMT_YUV444P;
        }
        codec_ctx_->thread_count = std::min(16, (int)std::thread::hardware_concurrency());
        codec_ctx_->thread_type = FF_THREAD_SLICE;
        codec_ctx_->gop_size = gop_size_;
        codec_ctx_->max_b_frames = 0;
        codec_ctx_->bit_rate = bitrate_;

        LOGI("ffmpeg encoder config:");
        LOGI("bitrate: {}", codec_ctx_->bit_rate);
        LOGI("format: {}", (config.codec_type == EVideoCodecType::kHEVC ? "HEVC" : "H264"));
        LOGI("refresh rate(fps): {}", encoder_config_.fps);
        LOGI("thread count: {}", codec_ctx_->thread_count);
        LOGI("gop size: {}", codec_ctx_->gop_size);

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

        auto ret = avcodec_open2(codec_ctx_, encoder, &param);
        if (ret != 0) {
            LOGE("avcodec_open2 error : {}", ret);
            return false;
        }

        frame_ = av_frame_alloc();
        frame_->width = codec_ctx_->width;
        frame_->height = codec_ctx_->height;
        frame_->format = codec_ctx_->pix_fmt;

        av_frame_get_buffer(frame_, 0);
        packet_ = av_packet_alloc();
        LOGI("Line 1: {} 2: {} 3: {}", frame_->linesize[0], frame_->linesize[1], frame_->linesize[2]);
        return true;
    }

    void FFmpegEncoder::Encode(const std::shared_ptr<Image>& image, uint64_t frame_index, const std::any& extra) {
        if (!image) {
            LOGE("image is nullptr!");
            return;
        }
		auto beg = TimeUtil::GetCurrentTimestamp();
        auto cap_video_frame = std::any_cast<CaptureVideoFrame>(extra);
        auto img_width = image->width;
        auto img_height = image->height;
        auto image_data = image->data;

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
        if (RawImageType::kI420 == image->raw_img_type_) {
            uv_size = img_width * img_height / 4;
        }
        else if (RawImageType::kI444 == image->raw_img_type_) {
            LOGI("RawImageType::kI444");
            uv_size = img_width * img_height;
        }
        memcpy(frame_->data[0], image_data->CStr(), y_size);
        memcpy(frame_->data[1], image_data->CStr() + y_size, uv_size);
        memcpy(frame_->data[2], image_data->CStr() + y_size + uv_size, uv_size);

        int send_result = avcodec_send_frame(codec_ctx_, frame_);

        //LOGI("avcodec_send_frame send_result: {}", send_result);

        while (send_result >= 0) {
            int receiveResult = avcodec_receive_packet(codec_ctx_, packet_);
            //LOGI("avcodec_receive_packet receiveResult: {}", receiveResult);
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
            event->frame_format_ = encoder_config_.enable_full_color_mode_ ? RawImageType::kI444 : RawImageType::kI420;
            plugin_->CallbackEvent(event);

            auto end = TimeUtil::GetCurrentTimestamp();
            auto diff = end - beg;
            if (encode_durations_.size() >= 180) {
                encode_durations_.pop_front();
            }
            encode_durations_.push_back((int32_t)diff);

            fps_stat_->Tick();

            av_packet_unref(packet_);
        }
    }

    void FFmpegEncoder::InsertIdr() {
        insert_idr_ = true;
    }

    void FFmpegEncoder::Exit() {
        av_packet_unref(packet_);
        av_frame_free(&frame_);
        if (codec_ctx_) {
            avcodec_close(codec_ctx_);
        }
        avcodec_free_context(&codec_ctx_);
    }

    int32_t FFmpegEncoder::GetEncodeFps() {
        return fps_stat_->value();
    }

    std::vector<int32_t> FFmpegEncoder::GetEncodeDurations() {
        std::vector<int32_t> result;
        for (const auto& item : encode_durations_) {
            result.push_back(item);
        }
        return result;
    }
	
    void FFmpegEncoder::InitLog() {
        std::call_once(init_log_flag_, []() {
            av_log_set_level(AV_LOG_WARNING);
            av_log_set_callback([](void* ptr, int level, const char* fmt, va_list vl)
                {
                    static int print_prefix = 1;
                    std::string line;
                    line.resize(4096);
                    av_log_format_line(ptr, level, fmt, vl, line.data(), line.size(), &print_prefix);
                    line = StringExt::Trim(line);
                    if (level <= AV_LOG_WARNING)
                        LOGI("ffmpeg_wlog:{}", line.c_str());
                }
            );
        });
    }
}
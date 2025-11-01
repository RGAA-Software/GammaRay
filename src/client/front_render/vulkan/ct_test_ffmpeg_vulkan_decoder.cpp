#include "ct_test_ffmpeg_vulkan_decoder.h"
#include <iostream>
#include <string>
#include <qstring.h>
#include <qdebug.h>
#include <chrono>
#include <thread>
#include "tc_common_new/log.h"

namespace tc {

    // 720p HEVC RExt8 4:4:4
    // ffmpeg -i green128.png -pix_fmt yuv444p -colorspace smpte170m -color_trc smpte170m -color_primaries smpte170m -c:v libx265 -x265-params info=0 test8_444.hevc
    // xxd -i test8_444.hevc
    const uint8_t TestFFmpegVulkanDecoder::k_HEVCRExt8_444TestFrame[] = {
        0x00, 0x00, 0x00, 0x01, 0x40, 0x01, 0x0c, 0x01, 0xff, 0xff, 0x04, 0x08,
        0x00, 0x00, 0x03, 0x00, 0x9e, 0x08, 0x00, 0x00, 0x03, 0x00, 0x00, 0x5d,
        0x95, 0x98, 0x09, 0x00, 0x00, 0x00, 0x01, 0x42, 0x01, 0x01, 0x04, 0x08,
        0x00, 0x00, 0x03, 0x00, 0x9e, 0x08, 0x00, 0x00, 0x03, 0x00, 0x00, 0x5d,
        0x90, 0x00, 0x50, 0x10, 0x05, 0xa2, 0xcb, 0x2b, 0x34, 0x92, 0x65, 0x78,
        0x0b, 0x50, 0x60, 0x60, 0x60, 0x40, 0x00, 0x00, 0x03, 0x00, 0x40, 0x00,
        0x00, 0x06, 0x42, 0x00, 0x00, 0x00, 0x01, 0x44, 0x01, 0xc1, 0x72, 0x86,
        0x0c, 0x46, 0x24, 0x00, 0x00, 0x01, 0x28, 0x01, 0xaf, 0x1d, 0x18, 0x69,
        0x57, 0x59, 0x55, 0x54, 0x51, 0x34, 0xd2, 0x4a, 0xf7, 0xcf, 0x80, 0xff,
        0xf1, 0xcc, 0x1f, 0xc9, 0x84, 0x7d, 0xf8, 0xb6, 0xba, 0xfa, 0xcd, 0x61,
        0xb5, 0xe3, 0xc1, 0x02, 0x19, 0x26, 0x30, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x04, 0xf4, 0xa8, 0x17,
        0x96, 0x03, 0x4c, 0x4e, 0x1a, 0x80, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
        0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x09, 0xf8, 0x93, 0x0b,
        0x93, 0x40, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
        0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x48, 0xc0, 0x87, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x03, 0x00, 0x01, 0xa3, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x03, 0x00, 0x00, 0xb5, 0x80, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
        0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
        0x0b, 0xd8, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
        0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x51, 0xc0, 0x00,
        0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
        0x00, 0x03, 0x00, 0x00, 0x03, 0x01, 0x39, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x03, 0x02, 0xca, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
        0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x04, 0x74, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x07, 0x6c, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
        0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x27, 0xa0,
    };

    enum AVPixelFormat ffGetFormat(AVCodecContext* context, const enum AVPixelFormat* pixFmts) {
        for (const AVPixelFormat* p = pixFmts; *p != AV_PIX_FMT_NONE; p++) {
            if (*p == AV_PIX_FMT_VULKAN)
                return *p;
        }
        return AV_PIX_FMT_NONE; // 如果 Vulkan 不在候选中，直接失败
    }

    std::shared_ptr<TestFFmpegVulkanDecoder> TestFFmpegVulkanDecoder::Make() {
        return std::make_shared<TestFFmpegVulkanDecoder>();
    }

	TestFFmpegVulkanDecoder::TestFFmpegVulkanDecoder() {

	}

	TestFFmpegVulkanDecoder::~TestFFmpegVulkanDecoder() {

	}

    void TestFFmpegVulkanDecoder::SetHwDeviceCtx(AVBufferRef* hw_device_ctx) {
        test_hevc_video_decoder_ctx_->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    }

    // test hevc
    bool TestFFmpegVulkanDecoder::InitTestHevcDecoder() {
        test_hevc_decoder_ = const_cast<AVCodec*>(avcodec_find_decoder(AV_CODEC_ID_HEVC));
        if (!test_hevc_decoder_) {
            qDebug() << "can not find decoder for AV_CODEC_ID_Hevc";
            return false;
        }

        test_hevc_video_decoder_ctx_ = avcodec_alloc_context3(test_hevc_decoder_);
        if (!test_hevc_video_decoder_ctx_) {
            qDebug() << "avcodec_alloc_context3 error";
            return false;
        }

        // Always request low delay decoding
        test_hevc_video_decoder_ctx_->flags |= AV_CODEC_FLAG_LOW_DELAY;

        // Allow display of corrupt frames and frames missing references
        test_hevc_video_decoder_ctx_->flags |= AV_CODEC_FLAG_OUTPUT_CORRUPT;
        test_hevc_video_decoder_ctx_->flags2 |= AV_CODEC_FLAG2_SHOW_ALL;

        // Report decoding errors to allow us to request a key frame
        //
        // With HEVC streams, FFmpeg can drop a frame (hwaccel->start_frame() fails)
        // without telling us. Since we have an infinite GOP length, this causes artifacts
        // on screen that persist for a long time. It's easy to cause this condition
        // by using NVDEC and delaying 100 ms randomly in the render path so the decoder
        // runs out of output buffers.
        test_hevc_video_decoder_ctx_->err_recognition = AV_EF_EXPLODE;

        test_hevc_video_decoder_ctx_->pix_fmt = AV_PIX_FMT_VULKAN;// 表示 解码输出的像素格式
        test_hevc_video_decoder_ctx_->get_format = ffGetFormat;// 是 FFmpeg 解码器在解码初始化阶段调用的回调函数，用来由你（应用层）选择最终的输出像素格式

        return true;
    }

    bool TestFFmpegVulkanDecoder::OpenTestHevcDecoder() {
        AVDictionary* options = nullptr;
        int err = avcodec_open2(test_hevc_video_decoder_ctx_, test_hevc_decoder_, &options);
        av_dict_free(&options);
        if (0 == err) {
            return true;
        }
        return false;
    }

    
    void TestFFmpegVulkanDecoder::FreeTestHevcYuv444Frame(AVFrame* frame) {
        av_frame_free(&frame);
    }

    std::optional<AVFrame*> TestFFmpegVulkanDecoder::GetDecodeTestHevcYuv444Frame() {
        AVFrame* frame = av_frame_alloc();
        if (!frame) {
            qDebug() << "av_frame_alloc error";
            return {};
        }

        AVPacket* pkt = av_packet_alloc();
        pkt->data = (uint8_t*)k_HEVCRExt8_444TestFrame;
        pkt->size = sizeof(k_HEVCRExt8_444TestFrame);

        int err = 0;

        // Some decoders won't output on the first frame, so we'll submit
        // a few test frames if we get an EAGAIN error.
        for (int retries = 0; retries < 5; retries++) {
            // Most FFmpeg decoders process input using a "push" model.
            // We'll see those fail here if the format is not supported.
            err = avcodec_send_packet(test_hevc_video_decoder_ctx_, pkt);
            if (err < 0) {
                av_frame_free(&frame);
                char errorstring[512];
                av_strerror(err, errorstring, sizeof(errorstring));
                printf("Test decode failed (avcodec_send_packet): %s", errorstring);
                return {};
            }

            // A few FFmpeg decoders (hevc_mmal) process here using a "pull" model.
            // Those decoders will fail here if the format is not supported.
            err = avcodec_receive_frame(test_hevc_video_decoder_ctx_, frame);
            if (err == AVERROR(EAGAIN)) {
                // Wait a little while to let the hardware work
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            else {
                // Done!
                break;
            }
        }

        if (err < 0) {
            char errorstring[512];
            av_strerror(err, errorstring, sizeof(errorstring));
            printf("Test decode failed (avcodec_receive_frame): %s", errorstring);
            av_frame_free(&frame);
            return {};
        }

        av_packet_free(&pkt);
        avcodec_free_context(&test_hevc_video_decoder_ctx_);


        LOGI("Frame format: {} {}",
            frame->format,
            av_get_pix_fmt_name((AVPixelFormat)frame->format));
        if (frame->hw_frames_ctx) {
            const AVHWFramesContext* hwfc = (const AVHWFramesContext*)frame->hw_frames_ctx->data;
            LOGI("HW device type: {}, sw_format: {}",
                av_hwdevice_get_type_name(hwfc->device_ctx->type),
                av_get_pix_fmt_name((AVPixelFormat)hwfc->sw_format));
        }
        else {
            LOGI("No hw_frames_ctx!\n");
        }

        return frame;
    }
}

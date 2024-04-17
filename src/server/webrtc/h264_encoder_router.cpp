//
// Created by RGAA on 2024-03-03.
//

#include "h264_encoder_router.h"
#include "tc_common_new/log.h"

namespace tc
{

    void H264EncoderRouter::SetFecControllerOverride(webrtc::FecControllerOverride *fec_controller_override) {
        VideoEncoder::SetFecControllerOverride(fec_controller_override);
    }

    int32_t H264EncoderRouter::InitEncode(const webrtc::VideoCodec *codec_settings, int32_t number_of_cores, size_t max_payload_size) {
        return VideoEncoder::InitEncode(codec_settings, number_of_cores, max_payload_size);
    }

    int H264EncoderRouter::InitEncode(const webrtc::VideoCodec *codec_settings, const webrtc::VideoEncoder::Settings &settings) {
        return VideoEncoder::InitEncode(codec_settings, settings);
    }

    int32_t H264EncoderRouter::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback *callback) {
        encoded_image_cbk_ = callback;
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int32_t H264EncoderRouter::Release() {
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int32_t H264EncoderRouter::Encode(const webrtc::VideoFrame &frame, const std::vector<webrtc::VideoFrameType> *frame_types) {
        LOGI("mock to encode frame...");
        return WEBRTC_VIDEO_CODEC_OK;
    }

    void H264EncoderRouter::SetRates(const webrtc::VideoEncoder::RateControlParameters &parameters) {

    }

    void H264EncoderRouter::OnPacketLossRateUpdate(float packet_loss_rate) {
        VideoEncoder::OnPacketLossRateUpdate(packet_loss_rate);
    }

    void H264EncoderRouter::OnRttUpdate(int64_t rtt_ms) {
        VideoEncoder::OnRttUpdate(rtt_ms);
    }

    void H264EncoderRouter::OnLossNotification(const webrtc::VideoEncoder::LossNotification &loss_notification) {
        VideoEncoder::OnLossNotification(loss_notification);
    }

    webrtc::VideoEncoder::EncoderInfo H264EncoderRouter::GetEncoderInfo() const {
        return VideoEncoder::GetEncoderInfo();
    }
}

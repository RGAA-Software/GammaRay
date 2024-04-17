//
// Created by RGAA on 2024-03-03.
//

#ifndef TC_APPLICATION_H264_ENCODER_ROUTER_H
#define TC_APPLICATION_H264_ENCODER_ROUTER_H

#include "webrtc_helper.h"

namespace tc
{

    class H264EncoderRouter : public webrtc::VideoEncoder {
    public:
        void SetFecControllerOverride(webrtc::FecControllerOverride *fec_controller_override) override;

        int32_t InitEncode(const webrtc::VideoCodec *codec_settings, int32_t number_of_cores, size_t max_payload_size) override;

        int InitEncode(const webrtc::VideoCodec *codec_settings, const Settings &settings) override;

        int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback *callback) override;

        int32_t Release() override;

        int32_t Encode(const webrtc::VideoFrame &frame, const std::vector<webrtc::VideoFrameType> *frame_types) override;

        void SetRates(const RateControlParameters &parameters) override;

        void OnPacketLossRateUpdate(float packet_loss_rate) override;

        void OnRttUpdate(int64_t rtt_ms) override;

        void OnLossNotification(const LossNotification &loss_notification) override;

        EncoderInfo GetEncoderInfo() const override;

    private:
        webrtc::EncodedImageCallback* encoded_image_cbk_ = nullptr;
    };

}

#endif //TC_APPLICATION_H264_ENCODER_ROUTER_H

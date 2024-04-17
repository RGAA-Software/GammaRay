//
// Created by RGAA on 2024-03-03.
//

#ifndef TC_APPLICATION_H264_VIDEO_ENCODER_FACTORY_H
#define TC_APPLICATION_H264_VIDEO_ENCODER_FACTORY_H

#include "webrtc_helper.h"

namespace tc
{

    class H264EncoderFactory : public webrtc::VideoEncoderFactory {
    public:
        H264EncoderFactory();
        ~H264EncoderFactory() override;
        std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
        std::vector<webrtc::SdpVideoFormat> GetImplementations() const override;
        CodecSupport QueryCodecSupport(const webrtc::SdpVideoFormat &format, absl::optional<std::string> scalability_mode) const override;
        std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat &format) override;
        std::unique_ptr<EncoderSelectorInterface> GetEncoderSelector() const override;

    private:
        std::vector<webrtc::SdpVideoFormat> supported_formats_;
    };

}

#endif //TC_APPLICATION_H264_VIDEO_ENCODER_FACTORY_H

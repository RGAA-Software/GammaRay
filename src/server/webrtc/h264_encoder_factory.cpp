//
// Created by RGAA on 2024-03-03.
//

#include "h264_encoder_factory.h"
#include "h264_encoder_router.h"

namespace tc
{

    H264EncoderFactory::H264EncoderFactory() {
        supported_formats_ = webrtc::SupportedH264Codecs();
        supported_formats_.push_back(webrtc::CreateH264Format(webrtc::H264Profile::kProfileBaseline, webrtc::H264Level::kLevel3_1, "1"));
        supported_formats_.push_back(webrtc::CreateH264Format(webrtc::H264Profile::kProfileBaseline, webrtc::H264Level::kLevel3_1, "0"));
        supported_formats_.push_back(webrtc::CreateH264Format(webrtc::H264Profile::kProfileConstrainedBaseline, webrtc::H264Level::kLevel3_1, "1"));
        supported_formats_.push_back(webrtc::CreateH264Format(webrtc::H264Profile::kProfileConstrainedBaseline, webrtc::H264Level::kLevel3_1, "0"));
        supported_formats_.push_back(webrtc::CreateH264Format(webrtc::H264Profile::kProfileMain, webrtc::H264Level::kLevel3_1, "1"));
        supported_formats_.push_back(webrtc::CreateH264Format(webrtc::H264Profile::kProfileMain, webrtc::H264Level::kLevel3_1, "0"));
    }

    std::vector<webrtc::SdpVideoFormat> H264EncoderFactory::GetSupportedFormats() const {
        return supported_formats_;
    }

    std::vector<webrtc::SdpVideoFormat> H264EncoderFactory::GetImplementations() const {
        return VideoEncoderFactory::GetImplementations();
    }

    webrtc::VideoEncoderFactory::CodecSupport H264EncoderFactory::QueryCodecSupport(const webrtc::SdpVideoFormat &format, absl::optional<std::string> scalability_mode) const {
        return VideoEncoderFactory::QueryCodecSupport(format, scalability_mode);
    }

    std::unique_ptr<webrtc::VideoEncoder> H264EncoderFactory::CreateVideoEncoder(const webrtc::SdpVideoFormat &format) {
        return std::make_unique<H264EncoderRouter>();
    }

    std::unique_ptr<webrtc::VideoEncoderFactory::EncoderSelectorInterface> H264EncoderFactory::GetEncoderSelector() const {
        return VideoEncoderFactory::GetEncoderSelector();
    }

    H264EncoderFactory::~H264EncoderFactory() {

    }

}

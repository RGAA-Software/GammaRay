//
// Created by RGAA on 2024/2/18.
//

#include "video_frame_observer.h"

namespace tc
{

    std::shared_ptr<VideoFrameObserver> VideoFrameObserver::Make(const RtcDecodedVideoFrame::RtcFrameType& frame_type, const OnFrameCallback& cbk) {
        return std::make_shared<VideoFrameObserver>(frame_type, cbk);
    }

    void VideoFrameObserver::OnFrame(const webrtc::VideoFrame &frame) {
        int rgb_buffer_size = frame.width() * frame.height() * 3;
        if (rgb_buffer_.size() != rgb_buffer_size) {
            rgb_buffer_.resize(rgb_buffer_size);
        }

        auto type = frame.video_frame_buffer()->type();
        if (type == webrtc::VideoFrameBuffer::Type::kI420) {
            auto result = ExtractBuffer(frame, rgb_buffer_size, rgb_buffer_.data());
            std::cout << "extract buffer size: " << result << std::endl;
        }

        auto ok = ConvertFromI420(frame, webrtc::VideoType::kRGB24, 0, rgb_buffer_.data()) == 0;
        if (ok) {
            auto decoded_frame = std::make_shared<RtcDecodedVideoFrame>();
            decoded_frame->frame_width_ = frame.width();
            decoded_frame->frame_height_ = frame.height();
            decoded_frame->buffer_.resize(rgb_buffer_size);
            memcpy(decoded_frame->buffer_.data(), rgb_buffer_.data(), rgb_buffer_size);
            frame_callback_(std::move(decoded_frame));
        }
    }

    void VideoFrameObserver::OnDiscardedFrame() {
        std::cout << "OnDiscardedFrame..." << std::endl;
    }

    void VideoFrameObserver::OnConstraintsChanged(const webrtc::VideoTrackSourceConstraints &constraints) {
        std::cout << "OnConstraint changed... " << std::endl;
    }

}

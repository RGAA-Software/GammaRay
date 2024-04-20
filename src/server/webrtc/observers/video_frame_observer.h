//
// Created by RGAA on 2024/2/4.
//

#ifndef TEST_WEBRTC_VIDEO_FRAME_OBSERVER_H
#define TEST_WEBRTC_VIDEO_FRAME_OBSERVER_H

#include "webrtc_helper.h"

#include <iostream>
#include <utility>

namespace tc
{

    class RtcDecodedVideoFrame {
    public:
        enum class RtcFrameType {
            kRtcFrameRGB24,
            kRtcFrameI420,
        };
    public:
        std::vector<uint8_t> buffer_;
        uint32_t frame_width_;
        uint32_t frame_height_;
    };

    using RtcDecodedVideoFramePtr = std::shared_ptr<RtcDecodedVideoFrame>;
    using OnFrameCallback = std::function<void(RtcDecodedVideoFramePtr&& frame)>;

    class VideoFrameObserver : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
    public:

        OnFrameCallback frame_callback_;
        std::vector<uint8_t> rgb_buffer_;

    public:

        static std::shared_ptr<VideoFrameObserver> Make(const RtcDecodedVideoFrame::RtcFrameType& frame_type, const OnFrameCallback& cbk);

        explicit VideoFrameObserver(const RtcDecodedVideoFrame::RtcFrameType& frame_type, const OnFrameCallback& cbk) :
            frame_type_(frame_type), frame_callback_(cbk) {
        }

        void OnFrame(const webrtc::VideoFrame& frame) override;
        void OnDiscardedFrame() override;
        void OnConstraintsChanged(const webrtc::VideoTrackSourceConstraints& constraints) override;
    private:
        RtcDecodedVideoFrame::RtcFrameType frame_type_;
    };

}

#endif //TEST_WEBRTC_VIDEO_FRAME_OBSERVER_H

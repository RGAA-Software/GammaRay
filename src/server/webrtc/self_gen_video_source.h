//
// Created by hy on 2024/3/4.
//

#ifndef TC_APPLICATION_VIDEO_SOURCE_H
#define TC_APPLICATION_VIDEO_SOURCE_H

#include "webrtc_helper.h"

namespace tc
{

    struct AdaptFrameResult
    {
        int cropped_width = 0;
        int cropped_height = 0;
        int width = 0;
        int height = 0;
        bool drop;
        bool resize;
    };

    class FramePreprocessor
    {
    public:
        virtual ~FramePreprocessor() = default;
        virtual webrtc::VideoFrame Preprocess(const webrtc::VideoFrame& frame) = 0;
    };

    class SelfGenVideoSource : public rtc::VideoSourceInterface<webrtc::VideoFrame> {
    public:
        void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink, const rtc::VideoSinkWants &wants) override;
        void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink) override;
        void AddFrame(const webrtc::VideoFrame& original_frame);

    private:
#if 0
        webrtc::VideoFrame MaybePreprocess(const webrtc::VideoFrame& frame);
        AdaptFrameResult AdaptFrameResolution(const webrtc::VideoFrame& frame);
        webrtc::VideoFrame ScaleFrame(const webrtc::VideoFrame& original_frame, AdaptFrameResult& ret);
#endif
        webrtc::Mutex _lock;
        rtc::VideoBroadcaster _broadcaster;
        //cricket::VideoAdapter _video_adapter;
        //std::unique_ptr<FramePreprocessor> _preprocessor RTC_GUARDED_BY(_lock);
    };

}

#endif //TC_APPLICATION_VIDEO_SOURCE_H

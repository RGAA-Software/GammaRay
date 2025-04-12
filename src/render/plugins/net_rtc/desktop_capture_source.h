//
// Created by hy on 20/07/2024.
//

#ifndef WEBRTC_CLIENT_DESKTOP_CAPTURE_SOURCE_H
#define WEBRTC_CLIENT_DESKTOP_CAPTURE_SOURCE_H

#include "tc_common_new/webrtc_helper.h"

#if 0
namespace dl
{

    class DesktopCaptureSource : public rtc::VideoSourceInterface<webrtc::VideoFrame> {
    public:
        void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink, const rtc::VideoSinkWants &wants) override;
        void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink) override;
        void OnFrame(const webrtc::VideoFrame& frame);

    private:
        void UpdateVideoAdapter();

        rtc::VideoBroadcaster broadcaster_;
        cricket::VideoAdapter video_adapter_;
    };

}
#endif
#endif //WEBRTC_CLIENT_DESKTOP_CAPTURE_SOURCE_H

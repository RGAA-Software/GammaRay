//
// Created by hy on 20/07/2024.
//

#ifndef WEBRTC_CLIENT_DESKTOP_CAPTURE_H
#define WEBRTC_CLIENT_DESKTOP_CAPTURE_H

#include "tc_common_new/webrtc_helper.h"
#include "desktop_capture_source.h"

namespace tc
{

    class DesktopCapture : public rtc::VideoSourceInterface<webrtc::VideoFrame>,
                           public webrtc::DesktopCapturer::Callback {
    public:
        static std::shared_ptr<DesktopCapture> Create(size_t target_fps, size_t capture_screen_index);
        ~DesktopCapture() override;
        void StartCapture();
        void StopCapture();

        void OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame) override;
        void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink, const rtc::VideoSinkWants &wants) override;
        void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink) override;

    private:
        DesktopCapture();

        bool Init(size_t target_fps, size_t capture_screen_index);
        void UpdateVideoAdapter();
        void BroadcastFrame(const webrtc::VideoFrame& frame);

    private:
        rtc::VideoBroadcaster broadcaster_;
        cricket::VideoAdapter video_adapter_;
        std::unique_ptr<webrtc::DesktopCapturer> rtc_desktop_capture_ = nullptr;
        size_t fps_{};
        std::unique_ptr<std::thread> capture_thread_;
        std::atomic_bool start_flag_;
        rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer_;
        uint64_t last_capture_callback_time_ = 0;
        int callback_fps_ = 0;
    };

}


#endif //WEBRTC_CLIENT_DESKTOP_CAPTURE_H

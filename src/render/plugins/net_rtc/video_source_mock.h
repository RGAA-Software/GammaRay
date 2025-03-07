//
// Created by hy on 2024/4/26.
//

#ifndef TEST_WEBRTC_VIDEO_SOURCE_MOCK_H
#define TEST_WEBRTC_VIDEO_SOURCE_MOCK_H

#include "webrtc_helper.h"
#include "i420_creator.h"
#include <fstream>
#include <chrono>
#include <thread>

namespace tc
{
    static int64_t cur_time() {
        int64_t time_cur = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        return time_cur;
    }

    static constexpr auto kFrameWidth = 1920;
    static constexpr auto kFrameHeight = 1080;

    class VideoSourceMock : public rtc::VideoSourceInterface<webrtc::VideoFrame> {
    public:
        VideoSourceMock() : i420_creator_(std::bind(&VideoSourceMock::on_frame, this, std::placeholders::_1)) {
            i420_creator_.set_resolution(kFrameWidth, kFrameHeight);
            i420_creator_.run();
        }

        void on_frame(I420Creator::I420Frame frame) {
            static int i = 0;
            //std::cout << "[info] sending frame, no:" << i++ << std::endl;

//            std::ofstream mock_frame("frame.yuv", std::ios::binary);
//            mock_frame.write(reinterpret_cast<const char *>(frame->data()), frame->size());
//            mock_frame.close();

            rtc::scoped_refptr<webrtc::I420Buffer> buffer =
                    webrtc::I420Buffer::Copy(kFrameWidth, kFrameHeight,
                                             frame->data(), kFrameWidth,
                                             frame->data() + kFrameWidth * kFrameHeight, kFrameWidth / 2,
                                             frame->data() + kFrameWidth * kFrameHeight + kFrameWidth * kFrameHeight / 4, kFrameWidth / 2);
            webrtc::VideoFrame captureFrame = webrtc::VideoFrame::Builder()
                    .set_video_frame_buffer(buffer)
                    .set_timestamp_rtp(0)
                    .set_timestamp_ms(rtc::TimeMillis())
                    .set_rotation(webrtc::kVideoRotation_0).build();
            captureFrame.set_ntp_time_ms(cur_time());
            //TODO:convert i420 to 'videoframe'
            broadcaster_.OnFrame(captureFrame);
        }

    private:
        void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink,
                             const rtc::VideoSinkWants &wants) override {
            broadcaster_.AddOrUpdateSink(sink, wants);
            (void) video_adapter_; //we willn't use adapter at this demo
        }

        void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink) override {
            broadcaster_.RemoveSink(sink);
            (void) video_adapter_; //we willn't use adapter at this demo
        }

    private:
        rtc::VideoBroadcaster broadcaster_;
        cricket::VideoAdapter video_adapter_;
        I420Creator i420_creator_;

    };


    ////

    class VideoTrack : public webrtc::VideoTrackSource {
    public:
        VideoTrack(const std::shared_ptr<rtc::VideoSourceInterface<webrtc::VideoFrame>>& video_source) : webrtc::VideoTrackSource(false) {
            video_source_ = video_source;
        }

    protected:
        rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
            return video_source_.get();
        }

    private:
        std::shared_ptr<rtc::VideoSourceInterface<webrtc::VideoFrame>> video_source_;
    };

    ////
    class VideoStreamReceiver : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
    public:
        void OnFrame(const webrtc::VideoFrame& frame) override {
            std::cout<<"[info] received a frame, id:"<<frame.id()<<std::endl;
        }
    };

}

#endif //TEST_WEBRTC_VIDEO_SOURCE_MOCK_H

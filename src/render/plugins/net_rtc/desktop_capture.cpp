//
// Created by hy on 20/07/2024.
//

#include "desktop_capture.h"

#include <memory>
#include "third_party/libyuv/include/libyuv.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_util.h"
#include "tc_common_new/time_util.h"

namespace tc
{

    DesktopCapture::DesktopCapture() : rtc_desktop_capture_(nullptr), start_flag_(false) {

    }

    DesktopCapture::~DesktopCapture() {

    }

    std::shared_ptr<DesktopCapture> DesktopCapture::Create(size_t target_fps, size_t capture_screen_index) {
        std::shared_ptr<DesktopCapture> dc(new DesktopCapture());
        if (!dc->Init(target_fps, capture_screen_index)) {
            LOGE("Failed to create DesktopCapture fps = {}, index: {}", target_fps, capture_screen_index);
            return nullptr;
        }
        LOGI("DesktopCapture init success.");
        return dc;
    }

    bool DesktopCapture::Init(size_t target_fps, size_t capture_screen_index) {
        auto options = webrtc::DesktopCaptureOptions::CreateDefault();
        options.set_allow_directx_capturer(true);
        rtc_desktop_capture_ = webrtc::DesktopCapturer::CreateScreenCapturer(options);
        rtc_desktop_capture_->SetMaxFrameRate(target_fps);
        if (!rtc_desktop_capture_) {
            LOGE("CreateScreenCapture failed!");
            return false;
        }

        webrtc::DesktopCapturer::SourceList sources;
        rtc_desktop_capture_->GetSourceList(&sources);
        LOGE("total screen : {}", sources.size());
        if (capture_screen_index > sources.size()) {
            LOGE("total screen : {}, bit you want to capture: {}", sources.size(), capture_screen_index);
            return false;
        }

        if (!rtc_desktop_capture_->SelectSource(sources[capture_screen_index].id)) {
            LOGE("Select souce failed,id: {}, title: {}", sources[capture_screen_index].id, sources[capture_screen_index].title);
            return false;
        }
        fps_ = target_fps;
         LOGI("Init DesktopCapture finish");
        return true;
    }

    void DesktopCapture::AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink, const rtc::VideoSinkWants& wants) {
        broadcaster_.AddOrUpdateSink(sink, wants);
        for (auto& item : wants.resolutions) {
            LOGI("item: {}x{}", item.width, item.height);
        }
        UpdateVideoAdapter();
    }

    void DesktopCapture::RemoveSink(
            rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) {
        broadcaster_.RemoveSink(sink);
        UpdateVideoAdapter();
    }


    void DesktopCapture::OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame) {
        if (result != webrtc::DesktopCapturer::Result::SUCCESS) {
            LOGE("Capture failed! {}", (int)result);
            return;
        }

        callback_fps_++;
        auto timestamp_curr = TimeUtil::GetCurrentTimestamp();
        if (timestamp_curr - last_capture_callback_time_ > 1000) {
            LOGI("FPS: {}", callback_fps_);
            callback_fps_ = 0;
            last_capture_callback_time_ = timestamp_curr;
        }

        int width = frame->size().width();
        int height = frame->size().height();

        if (!i420_buffer_.get() || i420_buffer_->width() * i420_buffer_->height() < width * height) {
            i420_buffer_ = webrtc::I420Buffer::Create(width, height);
        }

        libyuv::ConvertToI420(frame->data(), 0, i420_buffer_->MutableDataY(),
                              i420_buffer_->StrideY(), i420_buffer_->MutableDataU(),
                              i420_buffer_->StrideU(), i420_buffer_->MutableDataV(),
                              i420_buffer_->StrideV(), 0, 0, width, height, width,
                              height, libyuv::kRotate0, libyuv::FOURCC_ARGB);

        this->BroadcastFrame(webrtc::VideoFrame(i420_buffer_, 0, 0, webrtc::kVideoRotation_0));
    }

    void DesktopCapture::UpdateVideoAdapter() {
        video_adapter_.OnSinkWants(broadcaster_.wants());
    }

    void DesktopCapture::BroadcastFrame(const webrtc::VideoFrame& frame) {
#if 0
        int cropped_width = 0;
        int cropped_height = 0;
        int out_width = 0;
        int out_height = 0;

        if (!video_adapter_.AdaptFrameResolution(
                frame.width(), frame.height(), frame.timestamp_us() * 1000,
                &cropped_width, &cropped_height, &out_width, &out_height)) {
            // Drop frame in order to respect frame rate constraint.
            LOGE("Drop frame in order to respect frame rate constraint: cropped size: {}x{}, out size: {}x{}",
                         cropped_width, cropped_height, out_width, out_height);
            return;
        }

        if (out_height != frame.height() || out_width != frame.width()) {
            // Video adapter has requested a down-scale. Allocate a new buffer and
            // return scaled version.
            // For simplicity, only scale here without cropping.
            rtc::scoped_refptr<webrtc::I420Buffer> scaled_buffer = webrtc::I420Buffer::Create(out_width, out_height);
            scaled_buffer->ScaleFrom(*frame.video_frame_buffer()->ToI420());
            webrtc::VideoFrame::Builder new_frame_builder =
                    webrtc::VideoFrame::Builder()
                            .set_video_frame_buffer(scaled_buffer)
                            .set_rotation(webrtc::kVideoRotation_0)
                            .set_timestamp_us(frame.timestamp_us())
                            .set_id(frame.id());
            ;
            if (frame.has_update_rect()) {
                webrtc::VideoFrame::UpdateRect new_rect =
                        frame.update_rect().ScaleWithFrame(frame.width(), frame.height(), 0,
                                                           0, frame.width(), frame.height(),
                                                           out_width, out_height);
                new_frame_builder.set_update_rect(new_rect);
            }
            broadcaster_.OnFrame(new_frame_builder.build());
            LOGI("broadcaster_.OnFrame....");
        }
#endif
        broadcaster_.OnFrame(frame);

    }

    void DesktopCapture::StartCapture() {
        if (start_flag_) {
            LOGE("Desktop capture already started.");
            return;
        }
        start_flag_ = true;
        capture_thread_ = std::make_unique<std::thread>([this]() {
            rtc_desktop_capture_->Start(this);
            while (start_flag_) {
                auto beg = TimeUtil::GetCurrentTimestamp();
                rtc_desktop_capture_->CaptureFrame();
                auto end = TimeUtil::GetCurrentTimestamp();
                std::this_thread::sleep_for(std::chrono::milliseconds((1000 / fps_) - (end-beg)));
            }
        });
    }

    void DesktopCapture::StopCapture() {
        start_flag_ = false;
        if (capture_thread_ && capture_thread_->joinable()) {
            capture_thread_->join();
        }

        if (rtc_desktop_capture_) {
            rtc_desktop_capture_.reset(nullptr);
        }
    }

}
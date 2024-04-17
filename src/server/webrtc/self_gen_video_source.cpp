//
// Created by hy on 2024/3/4.
//

#include "self_gen_video_source.h"
#include "tc_common_new/log.h"

namespace tc
{

    void SelfGenVideoSource::AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink, const rtc::VideoSinkWants &wants) {
        _broadcaster.AddOrUpdateSink(sink, wants);
    }

    void SelfGenVideoSource::RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink) {
        _broadcaster.RemoveSink(sink);
    }

    void SelfGenVideoSource::AddFrame(const webrtc::VideoFrame& original_frame) {
        _broadcaster.OnFrame(original_frame);
        LOGI("broadcaster.OnFrame...");
//        auto frame = MaybePreprocess(original_frame);
//        auto ret = AdaptFrameResolution(frame);
//        if (!ret.drop) {
//            return;
//        }

//        if (ret.resize) {
//            _broadcaster.OnFrame(ScaleFrame(frame, ret));
//        } else {
//            _broadcaster.OnFrame(frame);
//        }
    }
#if 0
    webrtc::VideoFrame SelfGenVideoSource::MaybePreprocess(const webrtc::VideoFrame &frame) {
        webrtc::MutexLock lock(&_lock);
        if (_preprocessor != nullptr) {
            return _preprocessor->Preprocess(frame);
        } else {
            return frame;
        }
    }

    webrtc::VideoFrame SelfGenVideoSource::ScaleFrame(const webrtc::VideoFrame &original_frame, AdaptFrameResult &ret) {
        auto scaled_buffer = webrtc::I420Buffer::Create(ret.width, ret.height);
        scaled_buffer->ScaleFrom(*original_frame.video_frame_buffer()->ToI420());
        auto new_frame_builder = webrtc::VideoFrame::Builder()
                .set_video_frame_buffer(scaled_buffer)
                .set_rotation(webrtc::VideoRotation::kVideoRotation_0)
                .set_timestamp_us(original_frame.timestamp_us())
                .set_id(original_frame.id());

        if (!original_frame.has_update_rect()) {
            return new_frame_builder.build();
        }

        auto rect = original_frame.update_rect().ScaleWithFrame(
                original_frame.width(),
                original_frame.height(),
                0,
                0,
                original_frame.width(),
                original_frame.height(),
                ret.width,
                ret.height);
        new_frame_builder.set_update_rect(rect);
        return new_frame_builder.build();
    }

    AdaptFrameResult SelfGenVideoSource::AdaptFrameResolution(const webrtc::VideoFrame &frame) {
        AdaptFrameResult ret;
        ret.drop = _video_adapter.AdaptFrameResolution(
                frame.width(),
                frame.height(),
                frame.timestamp_us() * 1000,
                &ret.cropped_width,
                &ret.cropped_height,
                &ret.width,
                &ret.height);
        ret.resize = ret.height != frame.height() ||
                     ret.width != frame.width();
        return ret;
    }
#endif
}
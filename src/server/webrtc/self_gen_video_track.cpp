//
// Created by RGAA on 2024/3/4.
//

#include "self_gen_video_track.h"
#include "self_gen_video_source.h"

namespace tc
{

    SelfGenVideoTrack* SelfGenVideoTrack::Create() {
        auto self = new rtc::RefCountedObject<SelfGenVideoTrack>();
        self->AddRef();
        return self;
    }

    SelfGenVideoTrack::SelfGenVideoTrack() : webrtc::VideoTrackSource(false) {
        video_source_ = std::make_shared<SelfGenVideoSource>();
    }

    void SelfGenVideoTrack::AddFrame(const webrtc::VideoFrame& frame) {
        video_source_->AddFrame(frame);
    }

    rtc::VideoSourceInterface<webrtc::VideoFrame>* SelfGenVideoTrack::source() {
        return static_cast<rtc::VideoSourceInterface<webrtc::VideoFrame>*>(video_source_.get());
    }
}
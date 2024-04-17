//
// Created by hy on 2024/3/4.
//

#ifndef TC_APPLICATION_SELF_GEN_VIDEO_TRACK_H
#define TC_APPLICATION_SELF_GEN_VIDEO_TRACK_H

#include "webrtc_helper.h"

namespace tc
{

    class SelfGenVideoSource;

    class SelfGenVideoTrack : public webrtc::VideoTrackSource {
    public:

        static SelfGenVideoTrack* Create();

        SelfGenVideoTrack();
        void AddFrame(const webrtc::VideoFrame& frame);
        rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override;

    private:
        std::shared_ptr<SelfGenVideoSource> video_source_ = nullptr;
    };

}

#endif //TC_APPLICATION_SELF_GEN_VIDEO_TRACK_H

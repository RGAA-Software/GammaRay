//
// Created by RGAA on 2024/2/18.
//

#ifndef TEST_WEBRTC_CREATE_SESSION_DESC_OBSERVER_IMPL_H
#define TEST_WEBRTC_CREATE_SESSION_DESC_OBSERVER_IMPL_H

#include "webrtc_helper.h"

namespace tc
{

    class WebRtcServerImpl;

    class CreateSessionDescObserverImpl : public webrtc::CreateSessionDescriptionObserver {
    public:

        static rtc::scoped_refptr<CreateSessionDescObserverImpl> Make(const std::shared_ptr<WebRtcServerImpl>& client);
        explicit CreateSessionDescObserverImpl(const std::shared_ptr<WebRtcServerImpl>& client);

        // overrides
        void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;
        void OnFailure(webrtc::RTCError error) override;

    private:

        std::shared_ptr<WebRtcServerImpl> webrtc_client_ = nullptr;

    };
}

#endif //TEST_WEBRTC_CREATE_SESSION_DESC_OBSERVER_IMPL_H

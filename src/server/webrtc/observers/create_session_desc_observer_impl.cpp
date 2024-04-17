//
// Created by hy on 2024/2/18.
//

#include "create_session_desc_observer_impl.h"
#include "webrtc_server_impl.h"

namespace tc
{

    rtc::scoped_refptr<CreateSessionDescObserverImpl> CreateSessionDescObserverImpl::Make(const std::shared_ptr<WebRtcServerImpl>& client) {
        auto r =  new rtc::RefCountedObject<CreateSessionDescObserverImpl>(client);
        return rtc::scoped_refptr<CreateSessionDescObserverImpl>(r);
    }

    CreateSessionDescObserverImpl::CreateSessionDescObserverImpl(const std::shared_ptr<WebRtcServerImpl>& client) {
        webrtc_client_ = client;
    }

    void CreateSessionDescObserverImpl::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
        webrtc_client_->OnSessionCreated(desc);
    }

    void CreateSessionDescObserverImpl::OnFailure(webrtc::RTCError error) {

    }
}
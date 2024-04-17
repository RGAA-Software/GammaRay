//
// Created by hy on 2024/2/18.
//

#include "set_session_desc_observer_impl.h"

#include <iostream>

namespace tc
{

    rtc::scoped_refptr<SetSessionDescObserverImpl> SetSessionDescObserverImpl::Make(const std::shared_ptr<WebRtcServerImpl>& client) {
        auto c = new rtc::RefCountedObject<SetSessionDescObserverImpl>(client);
        return rtc::scoped_refptr<SetSessionDescObserverImpl>(c);
    }

    SetSessionDescObserverImpl::SetSessionDescObserverImpl(const std::shared_ptr<WebRtcServerImpl>& client) {
        webrtc_client_ = client;
    }

    void SetSessionDescObserverImpl::OnSuccess() {
        std::cout << "------> SetSessionDescObserverImpl::OnSuccess..." << std::endl;
    }

    void SetSessionDescObserverImpl::OnFailure(webrtc::RTCError error) {
        std::cout << "------> SetSessionDescObserverImpl::OnFailure..." << error.message() << std::endl;
    }

}
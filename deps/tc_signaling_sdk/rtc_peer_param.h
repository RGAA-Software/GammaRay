//
// Created by RGAA on 2024/6/2.
//

#ifndef TC_WEBRTC_CLIENT_RTC_PEER_PARAM_H
#define TC_WEBRTC_CLIENT_RTC_PEER_PARAM_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace tc {

class RtcDecodedVideoFrame {
public:
    enum class RtcFrameType {
		kRtcFrameRGB24,
		kRtcFrameI420,
		kRtcFrameYUY2,
    };
public:
    std::vector<uint8_t> buffer_;
    uint32_t frame_width_;
    uint32_t frame_height_;
    RtcFrameType frame_type_;
};

using RtcDecodedVideoFramePtr = std::shared_ptr<RtcDecodedVideoFrame>;
using OnFrameCallback = std::function<void(RtcDecodedVideoFramePtr&& frame)>;

class RtcPeerParam {
public:
    RtcDecodedVideoFrame::RtcFrameType frame_type_;
    std::string sig_host_;
    int sig_port_;
    std::string sig_path_;
    OnFrameCallback frame_callback_;
};

}

#endif //TC_WEBRTC_CLIENT_RTC_PEER_PARAM_H

//
// Created by RGAA on 2024-03-05.
//

#include "webrtc_server_api.h"
#include <memory>
#include <functional>
#include "webrtc_server_impl.h"
#include "tc_common_new/image.h"
#include "tc_common_new/data.h"

using namespace tc;

static std::shared_ptr<WebRtcServerImpl> s_webrtc_server_impl = nullptr;

int rtc_server_init(const rtc_server_param& param) {
    s_webrtc_server_impl = WebRtcServerImpl::Make();
    s_webrtc_server_impl->Init(WebRtcServerParam{});
    return RTC_SERVER_OK;
}

RTC_SERVER_API int rtc_server_send_video_frame(const char* buffer, size_t size, uint32_t frame_width, uint32_t frame_height, uint32_t format, uint64_t frame_idx, uint32_t key) {
    if (!s_webrtc_server_impl) {
        return RTC_SERVER_FAILED;
    }
    MsgVideoFrameEncoded msg{
        .frame_width_ = frame_width,
        .frame_height_ = frame_height,
        .frame_format_ = format,
        .frame_index_ = frame_idx,
        .key_frame_ = (bool)key,
        .image_ = Image::Make(Data::Make(buffer, (int)size), (int)frame_width, (int)frame_height),
    };
    s_webrtc_server_impl->OnImageEncoded(msg);
    return RTC_SERVER_OK;
}

void rtc_server_exit() {
    s_webrtc_server_impl->Exit();
}
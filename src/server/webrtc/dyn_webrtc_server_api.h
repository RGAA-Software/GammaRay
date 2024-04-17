//
// Created by RGAA on 2024-03-05.
//

#ifndef TC_APPLICATION_DYN_WEBRTC_SERVER_API_H
#define TC_APPLICATION_DYN_WEBRTC_SERVER_API_H

#include "webrtc_server_api.h"

#include <memory>
#include <functional>

using rtc_server_init_ptr_t = decltype(&rtc_server_init);
using rtc_server_exit_ptr_t = decltype(&rtc_server_exit);
using rtc_server_send_video_frame_ptr_t = decltype(&rtc_server_send_video_frame);

namespace tc
{

    class DynWebRtcServerApi {
    public:

        static std::shared_ptr<DynWebRtcServerApi> Make();

        void Start();
        void SendVideoFrame();
        void Exit();

    private:
        bool LoadWebrtcServer();

    private:

        rtc_server_init_ptr_t dyn_rtc_server_init_ = nullptr;
        rtc_server_exit_ptr_t dyn_rtc_server_exit_ = nullptr;
        rtc_server_send_video_frame_ptr_t dyn_rtc_server_send_video_frame_ = nullptr;

    };

}

#endif //TC_APPLICATION_DYN_WEBRTC_SERVER_API_H

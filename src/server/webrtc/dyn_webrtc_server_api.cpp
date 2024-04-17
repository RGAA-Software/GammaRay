//
// Created by RGAA on 2024-03-05.
//

#include "dyn_webrtc_server_api.h"
#include "tc_common_new/log.h"
#ifdef WIN32
#include <Windows.h>
#endif

namespace tc
{

    std::shared_ptr<DynWebRtcServerApi> DynWebRtcServerApi::Make() {
        return std::make_shared<DynWebRtcServerApi>();
    }

    void DynWebRtcServerApi::Start() {
        if (!LoadWebrtcServer()) {
            return;
        }

        if (dyn_rtc_server_init_(rtc_server_param{
            .priv_data = this,
        }) != RTC_SERVER_OK) {
            LOGE("rtc server init failed.");
            return;
        }
    }

    void DynWebRtcServerApi::SendVideoFrame() {

    }

    void DynWebRtcServerApi::Exit() {
        if (dyn_rtc_server_exit_) {
            dyn_rtc_server_exit_();
        }
    }


    bool DynWebRtcServerApi::LoadWebrtcServer() {
#ifdef WIN32
        auto module = LoadLibraryA("tc_rtc.dll");
        if (!module) {
            LOGE("load dlca_webrtc_clientmt.dll failed.");
            return false;
        }

        dyn_rtc_server_init_ = (rtc_server_init_ptr_t)GetProcAddress(module, "rtc_server_init");
        if (!dyn_rtc_server_init_) {
            LOGE("load rtc_server_init failed.");
            return false;
        }

        dyn_rtc_server_exit_ = (rtc_server_exit_ptr_t)GetProcAddress(module, "rtc_server_exit");
        if (!dyn_rtc_server_exit_) {
            LOGE("load rtc_server_exit failed.");
            return false;
        }

        dyn_rtc_server_send_video_frame_ = (rtc_server_send_video_frame_ptr_t)GetProcAddress(module, "rtc_server_send_video_frame");
        if (!dyn_rtc_server_send_video_frame_) {
            LOGE("load rtc_server_send_video_frame failed.");
            return false;
        }

        LOGE("load webrtc win client success.");
#endif
        return true;
    }

}
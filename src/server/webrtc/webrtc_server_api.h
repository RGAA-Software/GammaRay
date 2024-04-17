//
// Created by RGAA on 2024-03-05.
//

#ifndef TC_APPLICATION_WEBRTC_SERVER_API_H
#define TC_APPLICATION_WEBRTC_SERVER_API_H

#include <cstdint>

#if defined(_WIN32)
#if defined(__cplusplus)
#define RTC_SERVER_API extern "C" __declspec(dllexport)
#else
#define RTC_SERVER_API __declspec(dllexport)
#endif
#elif defined(__ANDROID__) || defined(__APPLE__)
#define DLCA_WEBRTC_API extern "C" __attribute__((visibility("default")))
#endif

struct rtc_server_param {
    void* priv_data = nullptr;
};

const int RTC_SERVER_OK = 0;
const int RTC_SERVER_FAILED = -1;

RTC_SERVER_API int rtc_server_init(const rtc_server_param& param);

RTC_SERVER_API int rtc_server_send_video_frame(const char* buffer, size_t size, uint32_t frame_width, uint32_t frame_height, uint32_t format, uint64_t frame_idx, uint32_t key);

RTC_SERVER_API void rtc_server_exit();

#endif //TC_APPLICATION_WEBRTC_SERVER_API_H

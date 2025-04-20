//
// Created by RGAA on 20/04/2025.
//

#ifndef GAMMARAY_RTC_REPORT_EVENT_H
#define GAMMARAY_RTC_REPORT_EVENT_H

#include <string>

namespace tc
{

    const std::string kDataChannelOpen = "data_channel_open";
    const std::string kDataChannelClose = "data_channel_close";
    const std::string kDataChannelSendError = "data_channel_send_error";
    const std::string kDataChannelRecvError = "data_channel_recv_error";

}


#endif //GAMMARAY_RTC_REPORT_EVENT_H

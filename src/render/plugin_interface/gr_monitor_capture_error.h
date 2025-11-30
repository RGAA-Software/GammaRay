//
// Created by RGAA on 30/11/2025.
//

#ifndef GAMMARAYPREMIUM_GR_MONITOR_CAPTURE_ERROR_H
#define GAMMARAYPREMIUM_GR_MONITOR_CAPTURE_ERROR_H

#include <functional>

namespace tc
{

    enum class MonitorCaptureError {
        kCantCapture, // capture failed, reinit failed
        kTimeoutSoManyTimes, // timeout so many times
    };

    using CaptureErrorCallback = std::function<void(const MonitorCaptureError& error)>;

}

#endif //GAMMARAYPREMIUM_GR_MONITOR_CAPTURE_ERROR_H

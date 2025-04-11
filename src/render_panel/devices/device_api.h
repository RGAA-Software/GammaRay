//
// Created by RGAA on 11/04/2025.
//

#ifndef GAMMARAY_DEVICE_API_H
#define GAMMARAY_DEVICE_API_H

#include <string>

namespace tc
{

    enum class DeviceVerifyResult {
        kVfEmptyDeviceId,
        kVfEmptyServerHost,
        kVfNetworkFailed,
        kVfResponseFailed,
        kVfParseJsonFailed,
        kVfSuccessRandomPwd,
        kVfSuccessSafetyPwd,
        kVfPasswordFailed,
    };

    class DeviceApi {
    public:
        // verify device_id/random_pwd pair
        // device id
        // random pwd: will be md5 in this func
        // safety pwd: will be md5 in this func
        static DeviceVerifyResult VerifyDeviceInfo(const std::string& device_id, const std::string& random_pwd, const std::string& safety_pwd);
    };

}

#endif //GAMMARAY_DEVICE_API_H

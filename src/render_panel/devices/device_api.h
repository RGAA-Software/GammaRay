//
// Created by RGAA on 11/04/2025.
//

#ifndef GAMMARAY_DEVICE_API_H
#define GAMMARAY_DEVICE_API_H

#include <string>

namespace tc
{

    enum class DeviceVerifyResult {
        kVfParamInvalid,
        kVfServerInternalError,
        kVfDeviceNotFound,
        kVfEmptyDeviceId,
        kVfEmptyServerHost,
        kVfNetworkFailed,
        kVfResponseFailed,
        kVfParseJsonFailed,
        kVfSuccessRandomPwd,
        kVfSuccessSafetyPwd,
        kVfPasswordFailed,
    };

    // HTTP CODE
    // see pr_error.rs
    constexpr int kERR_PARAM_INVALID = 600;
    constexpr int kERR_OPERATE_DB_FAILED = 601;
    constexpr int kERR_DEVICE_NOT_FOUND = 602;
    constexpr int kERR_PASSWORD_FAILED = 603;

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

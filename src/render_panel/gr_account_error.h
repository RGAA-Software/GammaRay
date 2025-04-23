//
// Created by RGAA on 23/04/2025.
//

#ifndef GAMMARAY_GR_ACCOUNT_ERROR_H
#define GAMMARAY_GR_ACCOUNT_ERROR_H

namespace tc
{

    enum class GrAccountError {
        kGrAccountOk,
        // device id is null
        kGrAccountDeviceInfoEmpty,
        // device id & device password not paired
        kGrAccountDeviceInfoNotPaired,
        kGrAccountEnd,
    };

    static std::string GrAccountError2String(const GrAccountError& err) {
        if (err == GrAccountError::kGrAccountOk) {
            return "ok";
        }
        else if (err == GrAccountError::kGrAccountDeviceInfoEmpty) {
            return "Device ID is empty";
        }
        else if (err == GrAccountError::kGrAccountDeviceInfoNotPaired) {
            return "Device ID & Password not paired";
        }
        else {
            return "unknown";
        }
    }

}

#endif //GAMMARAY_GR_ACCOUNT_ERROR_H

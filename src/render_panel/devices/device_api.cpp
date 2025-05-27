//
// Created by RGAA on 11/04/2025.
//

#include <format>
#include "device_api.h"
#include "render_panel/gr_settings.h"
#include "tc_common_new/http_client.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/log.h"
#include "tc_3rdparty/json/json.hpp"

using namespace nlohmann;

namespace tc
{

    ///verify/device/info
    DeviceVerifyResult DeviceApi::VerifyDeviceInfo(const std::string& device_id, const std::string& random_pwd, const std::string& safety_pwd) {
        auto settings = GrSettings::Instance();
        if (device_id.empty() || random_pwd.empty()) {
            return DeviceVerifyResult::kVfEmptyDeviceId;
        }
        if (settings->profile_server_host_.empty() || settings->profile_server_port_.empty()) {
            return DeviceVerifyResult::kVfEmptyServerHost;
        }
        auto client =
                HttpClient::Make(settings->profile_server_host_, std::atoi(settings->profile_server_port_.c_str()), "/verify/device/info", 2000);
        auto resp = client->Request({
            {"device_id", device_id},
            {"random_pwd_md5", random_pwd.empty() ? "" : MD5::Hex(random_pwd)},
            {"safety_pwd_md5", safety_pwd.empty() ? "" : MD5::Hex(safety_pwd)},
        });
        if (resp.status != 200 || resp.body.empty()) {
            LOGE("Request new device failed.");
            return DeviceVerifyResult::kVfNetworkFailed;
        }

        try {
            //LOGI("Verify resp: {}", resp.body);
            auto obj = json::parse(resp.body);
            auto code = obj["code"].get<int>();
            if (code == kERR_PARAM_INVALID) {
                return DeviceVerifyResult::kVfParamInvalid;
            }
            else if (code == kERR_OPERATE_DB_FAILED) {
                return DeviceVerifyResult::kVfServerInternalError;
            }
            else if (code == kERR_DEVICE_NOT_FOUND) {
                return DeviceVerifyResult::kVfDeviceNotFound;
            }
            else if (code == kERR_PASSWORD_FAILED) {
                return DeviceVerifyResult::kVfPasswordFailed;
            }
            else if (code != 200) {
                return DeviceVerifyResult::kVfPasswordFailed;
            }

            auto data = obj["data"];
            auto resp_device_id = data["device_id"].get<std::string>();
            auto pwd_type = data["pwd_type"].get<std::string>();
            //LOGI("Verify device info result: {}==>{}", resp_device_id, pwd_type);
            if (pwd_type == "random") {
                return DeviceVerifyResult::kVfSuccessRandomPwd;
            }
            else if (pwd_type == "safety") {
                return DeviceVerifyResult::kVfSuccessSafetyPwd;
            }
            else {
                return DeviceVerifyResult::kVfPasswordFailed;
            }
        } catch(...) {
            return DeviceVerifyResult::kVfParseJsonFailed;
        }
    }

}
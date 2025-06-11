//
// Created by RGAA on 11/06/2025.
//

#include "render_api.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/http_client.h"
#include "tc_common_new/log.h"
#include "tc_3rdparty/json/json.hpp"

using namespace nlohmann;

namespace tc
{

    const std::string kApiVerifySafetyPassword = "/verify/safety/password";

    Result<bool, int> RenderApi::VerifySafetyPassword(const std::string& host, int port, const std::string& safety_pwd_md5) {
        auto client = HttpClient::MakeSSL(host, port, kApiVerifySafetyPassword);
        auto r = client->Request({{
            "safety_pwd_md5", safety_pwd_md5
        }});

        LOGI("code: {}, msg: {}", r.status, r.body);
        if (r.status != 200 || r.body.empty()) {
            return ErrInt<bool>(r.status);
        }

        try {
            auto obj = json::parse(r.body);
            if (obj["code"].get<int>() == 200) {
                return true;
            }
        } catch(std::exception& e) {
            LOGE("Parse json failed: {}, body: {}", e.what(), r.body);
        }

        return ErrInt<bool>(-1);;
    }

}
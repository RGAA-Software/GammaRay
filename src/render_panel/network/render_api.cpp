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

    const std::string kApiVerifySecurityPassword = "/verify/security/password";
    const std::string kApiGetRenderConfiguration = "/get/render/configuration";

    Result<RenderConfiguration, int> RenderApi::GetRenderConfiguration(const std::string& host, int port) {
        auto client = HttpClient::MakeSSL(host, port, kApiGetRenderConfiguration);
        auto r = client->Request({});

        LOGI("code: {}, msg: {}", r.status, r.body);
        if (r.status != 200 || r.body.empty()) {
            return ErrInt<RenderConfiguration>(r.status);
        }

        try {
            auto obj = json::parse(r.body);
            if (obj["code"].get<int>() == 200) {
                RenderConfiguration rc;
                rc.device_id_ = obj["data"]["device_id"].get<std::string>();
                rc.relay_host_ = obj["data"]["relay_host"].get<std::string>();
                rc.relay_port_ = obj["data"]["relay_port"].get<int>();
                return rc;
            }
        } catch(std::exception& e) {
            LOGE("Parse json failed: {}, body: {}", e.what(), r.body);
        }

        return ErrInt<RenderConfiguration>(-1);
    }

    Result<bool, int> RenderApi::VerifySecurityPassword(const std::string& host, int port, const std::string& safety_pwd_md5) {
        auto client = HttpClient::MakeSSL(host, port, kApiVerifySecurityPassword);
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

        return ErrInt<bool>(-1);
    }

}
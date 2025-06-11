//
// Created by RGAA on 2023/12/20.
//
#include "http_handler.h"
#include "tc_common_new/log.h"
#include "tc_common_new/md5.h"
#include "rd_app.h"
#include "ws_plugin.h"

namespace tc
{

    constexpr auto kHandlerErrVerifySafetyPasswordFailed = 700;
    constexpr auto kHandlerErrNoSafetyPasswordInRenderer = 701;

    HttpHandler::HttpHandler(WsPlugin* plugin) {
        this->plugin_ = plugin;
    }

    std::string HttpHandler::GetErrorMessage(int code) {
        if (code == kHandlerErrVerifySafetyPasswordFailed) {
            return "Verify safety password failed";
        }
        else if (code == kHandlerErrNoSafetyPasswordInRenderer) {
            return "No safety password in renderer";
        }
        return BaseHandler::GetErrorMessage(code);
    }

    void HttpHandler::HandlePing(http::web_request &req, http::web_response &resp) {
        auto data = WrapBasicInfo(200, "ok", std::string("Pong"));
        resp.fill_json(data);
    }

    void HttpHandler::HandleVerifySafetyPassword(http::web_request& req, http::web_response& resp) {
        auto params = GetQueryParams(req.query());
        auto value = GetParam(params, "safety_pwd_md5");
        if (!value.has_value()) {
            SendErrorJson(resp, kHandlerErrParams);
            return;
        }

        auto settings = plugin_->GetSystemSettings();
        const auto& safety_pwd_md5 = value.value();
        if (safety_pwd_md5.empty() && !settings.device_safety_pwd_.empty()) {
            SendErrorJson(resp, kHandlerErrParams);
            return;
        }

        LOGI("request param, safety pwd md5: {}", safety_pwd_md5);
        if (settings.device_safety_pwd_.empty()) {
            SendOkJson(resp, "");
            //SendErrorJson(resp, kHandlerErrNoSafetyPasswordInRenderer);
            return;
        }

        LOGI("device safety pwd : {}", settings.device_safety_pwd_);
        if (settings.device_safety_pwd_ != safety_pwd_md5) {
            SendErrorJson(resp, kHandlerErrVerifySafetyPasswordFailed);
        }
        else {
            SendOkJson(resp, "");
        }
    }
}
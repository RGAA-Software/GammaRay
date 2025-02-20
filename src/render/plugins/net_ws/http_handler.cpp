//
// Created by RGAA on 2023/12/20.
//
#include "http_handler.h"
#include "tc_common_new/log.h"
#include "app.h"

namespace tc
{

    HttpHandler::HttpHandler(WsPlugin* plugin) {
        this->plugin_ = plugin;
    }

    void HttpHandler::HandlePing(http::web_request &req, http::web_response &rep) {
        auto data = WrapBasicInfo(200, "ok", std::string("Pong"));
        rep.fill_json(data);
    }

    std::string HttpHandler::WrapBasicInfo(int code, const std::string& msg, const std::string& data) {
        json obj;
        obj["code"] = code;
        obj["message"] = msg;
        try {
            obj["data"] = json::parse(data);
        } catch(...) {
            obj["data"] = data;
        }
        return obj.dump();
    }

    std::string HttpHandler::WrapBasicInfo(int code, const std::string& msg, const json& data) {
        json obj;
        obj["code"] = code;
        obj["message"] = msg;
        obj["data"] = data;
        return obj.dump();
    }
}
//
// Created by hy on 2023/12/20.
//
#include "apis.h"
#include "http_handler.h"
#include "tc_common/log.h"

namespace tc
{

    HttpHandler::HttpHandler(const std::shared_ptr<Context>& ctx) {
        this->context_ = ctx;
    }

    void HttpHandler::HandleSupportApis(const httplib::Request& req, httplib::Response& res) {
        res.set_content("Good", "text/plain");
    }

    void HttpHandler::HandleGames(const httplib::Request& req, httplib::Response& res) {
        res.set_content(R"({"aaa":"bbb"})", "application/json");
    }

    void HttpHandler::HandleGameStart(const httplib::Request& req, httplib::Response& res) {

    }

    void HttpHandler::HandleGameStop(const httplib::Request& req, httplib::Response& res) {

    }

}
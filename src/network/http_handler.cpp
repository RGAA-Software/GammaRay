//
// Created by hy on 2023/12/20.
//
#include "apis.h"
#include "http_handler.h"
#include "tc_common/log.h"

namespace tc
{

    // GET
    void HttpHandler::HandleSupportApis(uWS::HttpResponse<false> *res, uWS::HttpRequest* req) {
        LOGI("HandleSupportApis: {}", req->getFullUrl());
//        res->writeHeader("Content-Type", "application/json");
        res->end(GetSupportedApis());
    }

    // POST
    void HttpHandler::HandleReportInfo(uWS::HttpResponse<false> *res, uWS::HttpRequest* req) {
        LOGI("HandleReportInfo: {}", req->getFullUrl());
        res->end("YES>>>");
    }
}
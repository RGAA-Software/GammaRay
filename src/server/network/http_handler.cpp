//
// Created by hy on 2023/12/20.
//
#include "http_handler.h"
#include "tc_common_new/log.h"
#include "app.h"

namespace tc
{

    HttpHandler::HttpHandler(const std::shared_ptr<Application>& app) {
        this->app_ = app;
    }

    void HttpHandler::HandleServerState(http::web_request &req, http::web_response &rep) {

    }

    void HttpHandler::HandleWebRtcSdpRequest(http::web_request &req, http::web_response &rep) {

    }

}
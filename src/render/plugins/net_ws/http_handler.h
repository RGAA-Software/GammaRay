//
// Created by RGAA on 2023/12/20.
//

#ifndef TC_APPLICATION_HTTP_HANDLER_H
#define TC_APPLICATION_HTTP_HANDLER_H

#include <asio2/asio2.hpp>
#include "tc_3rdparty/json/json.hpp"
#include "tc_common_new/base_handler.h"

using namespace nlohmann;

namespace tc
{

    class WsPlugin;

    class HttpHandler : public BaseHandler {
    public:
        explicit HttpHandler(WsPlugin* plugin);
        std::string GetErrorMessage(int code) override;

        // /api/ping
        void HandlePing(http::web_request &req, http::web_response &rep);

        // /verify/security/password
        void HandleVerifySecurityPassword(http::web_request& req, http::web_response& rep);

        // /get/render/configuration
        void HandleGetRenderConfiguration(http::web_request& req, http::web_response& rep);

    private:
        WsPlugin* plugin_ = nullptr;

    };

}

#endif //TC_APPLICATION_HTTP_HANDLER_H

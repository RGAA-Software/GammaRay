//
// Created by RGAA on 2023/12/20.
//

#ifndef TC_APPLICATION_HTTP_HANDLER_H
#define TC_APPLICATION_HTTP_HANDLER_H

#include <asio2/asio2.hpp>
#include "tc_3rdparty/json/json.hpp"

using namespace nlohmann;

namespace tc
{

    class WsPlugin;

    class HttpHandler {
    public:
        explicit HttpHandler(WsPlugin* plugin);
        void HandlePing(http::web_request &req, http::web_response &rep);

    private:
        std::string WrapBasicInfo(int code, const std::string& msg, const std::string& data);
        std::string WrapBasicInfo(int code, const std::string& msg, const json& data);

    private:
        WsPlugin* plugin_ = nullptr;

    };

}

#endif //TC_APPLICATION_HTTP_HANDLER_H

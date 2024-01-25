//
// Created by hy on 2023/12/20.
//

#ifndef TC_APPLICATION_HTTP_HANDLER_H
#define TC_APPLICATION_HTTP_HANDLER_H

#include "http/httplib.h"

namespace tc
{

    class Context;

    class HttpHandler {
    public:

        explicit HttpHandler(const std::shared_ptr<Context>& ctx);

        void HandleSupportApis(const httplib::Request& req, httplib::Response& res);
        void HandleGames(const httplib::Request& req, httplib::Response& res);
        void HandleGameStart(const httplib::Request& req, httplib::Response& res);
        void HandleGameStop(const httplib::Request& req, httplib::Response& res);

    private:

        std::shared_ptr<Context> context_ = nullptr;

    };

}

#endif //TC_APPLICATION_HTTP_HANDLER_H

//
// Created by hy on 2023/12/20.
//

#ifndef TC_APPLICATION_HTTP_HANDLER_H
#define TC_APPLICATION_HTTP_HANDLER_H

#include <uwebsockets/App.h>

namespace tc
{

    class HttpHandler {
    public:

        // GET
        void HandleSupportApis(uWS::HttpResponse<false> *res, uWS::HttpRequest* req);

        // POST
        void HandleReportInfo(uWS::HttpResponse<false> *res, uWS::HttpRequest* req);
    };

}

#endif //TC_APPLICATION_HTTP_HANDLER_H

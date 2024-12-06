//
// Created by RGAA on 2023/12/20.
//

#ifndef TC_APPLICATION_HTTP_HANDLER_H
#define TC_APPLICATION_HTTP_HANDLER_H

#include <asio2/asio2.hpp>

namespace tc
{

    class Application;
    class Context;

    class HttpHandler {
    public:
        explicit HttpHandler(const std::shared_ptr<Application>& app);

        void HandleServerState(http::web_request &req, http::web_response &rep);
        void HandleWebRtcSdpRequest(http::web_request &req, http::web_response &rep);

    private:

        std::shared_ptr<Application> app_ = nullptr;

    };

}

#endif //TC_APPLICATION_HTTP_HANDLER_H

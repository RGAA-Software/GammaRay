#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "tc_3rdparty/http/httplib.h"

#include <thread>

namespace tc
{

    class HttpServer
    {
    public:
        HttpServer();
        ~HttpServer();

        void Start();
        void Exit();

    private:

        // HTTPS
        std::shared_ptr<httplib::SSLServer> server_ = nullptr;

        std::thread server_thread_;

    };

}

#endif // HTTP_SERVER_H

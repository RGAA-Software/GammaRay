#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <thread>
#include <memory>

namespace httplib {
    class Server;
}

namespace tc
{

    class Context;
    class HttpHandler;
    class Application;

    class HttpServer
    {
    public:
        explicit HttpServer(const std::shared_ptr<Application>& app);
        ~HttpServer();

        void Start();
        void Exit();

    private:

        std::shared_ptr<Context> context_ = nullptr;
        std::shared_ptr<Application> app_ = nullptr;
        std::shared_ptr<httplib::Server> server_ = nullptr;
        std::shared_ptr<HttpHandler> http_handler_ = nullptr;

        std::thread server_thread_;

    };

}

#endif // HTTP_SERVER_H

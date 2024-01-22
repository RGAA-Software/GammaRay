#include "http_server.h"

namespace tc
{

    HttpServer::HttpServer() {

    }

    HttpServer::~HttpServer() {

    }

    void HttpServer::Start() {
        server_thread_ = std::thread([=]() {
            server_ = std::make_shared<httplib::SSLServer>("./certificate.pem", "./private.key");
            server_->Get("/hi", [](const httplib::Request &, httplib::Response& res) {
                res.set_content("Hello World!", "text/plain");
            });

            auto ret = server_->set_mount_point("/public", "./htmls");

            server_->listen("0.0.0.0", 8080);
        });

    }

    void HttpServer::Exit() {
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

}

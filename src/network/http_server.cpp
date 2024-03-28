#include "http_server.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "tc_3rdparty/http/httplib.h"

#include "http_handler.h"

using namespace std::placeholders;

namespace tc
{

    HttpServer::HttpServer(const std::shared_ptr<Context>& ctx) {
        http_handler_ = std::make_shared<HttpHandler>(ctx);
    }

    HttpServer::~HttpServer() {

    }

    void HttpServer::Start() {
        server_thread_ = std::thread([this]() {
            server_ = std::make_shared<httplib::SSLServer>("./certificate.pem", "./private.key");
            server_->Get("/v1/apis", std::bind(&HttpHandler::HandleSupportApis, http_handler_.get(), _1, _2));
            server_->Get("/v1/games", std::bind(&HttpHandler::HandleGames, http_handler_.get(), _1, _2));
            server_->Get("/v1/game/start", std::bind(&HttpHandler::HandleGameStart, http_handler_.get(), _1, _2));
            server_->Get("/v1/game/stop", std::bind(&HttpHandler::HandleGameStop, http_handler_.get(), _1, _2));

            server_->set_mount_point("/", "./www");

            server_->listen("0.0.0.0", 20368);
        });

    }

    void HttpServer::Exit() {
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

}

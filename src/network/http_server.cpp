#include "http_server.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "tc_3rdparty/http/httplib.h"

#include "http_handler.h"
#include "context.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_common_new/log.h"
#include "application.h"
#include "settings.h"

#include <QString>
#include <QApplication>

using namespace std::placeholders;

namespace tc
{

    HttpServer::HttpServer(const std::shared_ptr<Application>& app) {
        context_ = app->GetContext();
        app_ = app;
        http_handler_ = std::make_shared<HttpHandler>(app);
    }

    HttpServer::~HttpServer() {

    }

    void HttpServer::Start() {
        server_thread_ = std::thread([this]() {
            //server_ = std::make_shared<httplib::SSLServer>("./certificate.pem", "./private.key");
            server_ = std::make_shared<httplib::Server>();
            server_->Get("/v1/ping", std::bind(&HttpHandler::HandlePing, http_handler_.get(), _1, _2));
            server_->Get("/v1/apis", std::bind(&HttpHandler::HandleSupportApis, http_handler_.get(), _1, _2));
            server_->Get("/v1/apps", std::bind(&HttpHandler::HandleGames, http_handler_.get(), _1, _2));
            server_->Get("/v1/start/app", std::bind(&HttpHandler::HandleGameStart, http_handler_.get(), _1, _2));
            server_->Get("/v1/stop/app", std::bind(&HttpHandler::HandleGameStop, http_handler_.get(), _1, _2));

            server_->set_mount_point("/", "./www");
            auto steam_manager = context_->GetSteamManager();
            if (steam_manager) {
                auto image_cache_path = steam_manager->GetSteamImageCachePath();
                server_->set_mount_point("/cache", image_cache_path);
            }

            auto res_path = QApplication::applicationDirPath() +  "/resources";
            server_->set_mount_point("/res", res_path.toStdString());

            server_->listen("0.0.0.0", Settings::Instance()->http_server_port_);
        });

    }

    void HttpServer::Exit() {
        if (server_) {
            server_->stop();
        }
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

}

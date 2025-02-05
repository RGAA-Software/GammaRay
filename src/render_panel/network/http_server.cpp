#include "http_server.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "tc_3rdparty/http/httplib.h"

#include "http_handler.h"
#include "render_panel/gr_context.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_common_new/log.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_settings.h"

#include <QString>
#include <QApplication>

using namespace std::placeholders;

namespace tc
{

    HttpServer::HttpServer(const std::shared_ptr<GrApplication> &app) {
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
            // response a "Pong" for checking server state
//            server_->Get(kPathPing, [=, this](const auto &req, auto &res) {
//                http_handler_->HandlePing(req, res);
//            });

            // response the information that equals to the QR Code
//            server_->Get(kPathSimpleInfo, [=, this](const auto &req, auto &res) {
//                http_handler_->HandleSimpleInfo(req, res);
//            });

            // response all apps that we found in system and added by user
//            server_->Get(kPathGames, [=, this](const auto &req, auto &res) {
//                http_handler_->HandleGames(req, res);
//            });

            // start game
//            server_->Post(kPathGameStart, [=, this](const auto &req, auto &res) {
//                http_handler_->HandleGameStart(req, res);
//            });

            // stop game
//            server_->Post(kPathGameStop, [=, this](const auto &req, auto &res) {
//                http_handler_->HandleGameStop(req, res);
//            });

            // running games
//            server_->Get(kPathRunningGames, [=, this](const auto &req, auto &res) {
//                http_handler_->HandleRunningGames(req, res);
//            });

            // stop the GammaRayRender.exe
//            server_->Get(kPathStopServer, [=, this](const auto& req, auto& res) {
//                http_handler_->HandleStopServer(req, res);
//            });

            // all running processes in th PC, equals the process list in TaskManager
//            server_->Get(kPathAllRunningProcesses, [=, this](const auto& req, auto& res) {
//                http_handler_->HandleAllRunningProcesses(req, res);
//            });

            // kill a process by pid
//            server_->Post(kPathKillProcess, [=, this](const auto& req, auto& res) {
//                http_handler_->HandleKillProcess(req, res);
//            });

            server_->set_mount_point("/", "./www");
            auto steam_manager = context_->GetSteamManager();
            if (steam_manager) {
                auto image_cache_path = steam_manager->GetSteamImageCachePath();
                LOGI("====> image cache path: {}", image_cache_path);
                server_->set_mount_point("/cache", image_cache_path);
            }

            auto exeDir = QApplication::applicationDirPath();
            server_->set_mount_point("/res", std::format("{}/resources", exeDir.toStdString()));
            server_->set_mount_point("/clients", std::format("{}/clients", exeDir.toStdString()));

            server_->listen("0.0.0.0", GrSettings::Instance()->http_server_port_);
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
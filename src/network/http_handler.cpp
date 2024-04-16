//
// Created by hy on 2023/12/20.
//
#include "apis.h"
#include "http_handler.h"
#include "tc_common_new/log.h"
#include "context.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_steam_manager_new/steam_entities.h"
#include "tc_3rdparty/json/json.hpp"
#include "application.h"
#include "app/tc_app_manager.h"
#include "tc_common_new/net_resp.h"
#include "apis.h"

using namespace nlohmann;

namespace tc
{

    HttpHandler::HttpHandler(const std::shared_ptr<Application>& app) {
        this->context_ = app->GetContext();
        this->app_ = app;
    }

    void HttpHandler::HandlePing(const httplib::Request &req, httplib::Response &res) {
        res.set_content("Pong", "text/plain");
    }

    void HttpHandler::HandleSimpleInfo(const httplib::Request &req, httplib::Response &res) {
        auto info = this->context_->MakeBroadcastMessage();
        auto resp = WrapBasicInfo(200, "ok", info);
        res.set_content(resp, "application/json");
    }

    void HttpHandler::HandleSupportApis(const httplib::Request& req, httplib::Response& res) {
        res.set_content("Good", "text/plain");
    }

    void HttpHandler::HandleGames(const httplib::Request& req, httplib::Response& res) {
        auto games_json_info = GetInstalledGamesAsJson();
        res.set_content(games_json_info, "application/json");
    }

    void HttpHandler::HandleGameStart(const httplib::Request& req, httplib::Response& res) {
        auto app_mgr = app_->GetAppManager();
        auto resp = app_mgr->Start("");
        NetResp nr = NetResp::Make(resp.ok_ ? kNetOk : kStartFailed, "start app failed", "");
        res.set_content(nr.Dump(), "application/json");
    }

    void HttpHandler::HandleGameStop(const httplib::Request& req, httplib::Response& res) {

    }

    void HttpHandler::HandleGameStartOnly(const httplib::Request& req, httplib::Response& res) {

    }

    void HttpHandler::HandleGameStopOnly(const httplib::Request& req, httplib::Response& res) {

    }

    // impl
    std::string HttpHandler::GetInstalledGamesAsJson() {
        auto games = context_->GetSteamManager()->GetInstalledGames();
        json obj;
        obj["code"] = 200;
        obj["message"] = "ok";
        json game_array = json::array();
        for (const auto& game : games) {
            json game_obj;
            game_obj["app_id"] = game->app_id_;
            game_obj["name"] = game->name_;
            game_obj["cover_name"] = game->cover_name_;
            game_obj["engine"] = game->engine_type_;
            game_array.push_back(game_obj);
        }
        obj["data"] = game_array;
        return obj.dump();
    }

    std::string HttpHandler::WrapBasicInfo(int code, const std::string& msg, const std::string& data) {
        json obj;
        obj["code"] = code;
        obj["message"] = msg;
        obj["data"] = json::parse(data);
        return obj.dump();
    }

    std::string HttpHandler::WrapBasicInfo(int code, const std::string& msg, const json& data) {
        json obj;
        obj["code"] = code;
        obj["message"] = msg;
        obj["data"] = data;
        return obj.dump();
    }

}
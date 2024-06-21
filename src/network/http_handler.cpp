//
// Created by RGAA on 2023/12/20.
//
#include "http_handler.h"
#include "apis.h"
#include "tc_common_new/log.h"
#include "gr_context.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_steam_manager_new/steam_entities.h"
#include "tc_3rdparty/json/json.hpp"
#include "gr_application.h"
#include "src/gr_server_manager.h"
#include "tc_common_new/net_resp.h"
#include "tc_3rdparty/json/json.hpp"
#include "src/gr_run_game_manager.h"

#include <boost/algorithm/string/trim.hpp>

using namespace nlohmann;

namespace tc
{

    HttpHandler::HttpHandler(const std::shared_ptr<GrApplication>& app) {
        this->context_ = app->GetContext();
        this->app_ = app;
        this->run_game_mgr_ = app->GetContext()->GetRunGameManager();
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
        res.set_content(GetSupportedApis(), "text/plain");
    }

    void HttpHandler::HandleGames(const httplib::Request& req, httplib::Response& res) {
        auto games_json_info = GetInstalledGamesAsJson();
        res.set_content(games_json_info, "application/json");
    }

    void HttpHandler::HandleGameStart(const httplib::Request& req, httplib::Response& res) {
        LOGI("body: {}", req.body);
        std::string game_path;
        try {
            auto obj = nlohmann::json::parse(req.body);
            game_path = obj["game_path"].get<std::string>();
            boost::trim(game_path);
            if (game_path.empty()) {
                LOGE("game path is empty");
                return;
            }
        } catch (std::exception& e) {
            LOGE("parse json failed, body:{}", req.body);
            return;
        }

        auto start_result = this->run_game_mgr_->StartGame(game_path, {});
        NetResp nr;
        if (start_result.ok_) {
            nr = NetResp::Make(kNetOk, "OK", "");
        } else {
            nr = NetResp::Make(kStartFailed, "start app failed", "");
        }
        res.set_content(nr.Dump(), "application/json");
    }

    void HttpHandler::HandleGameStop(const httplib::Request& req, httplib::Response& res) {

    }

    void HttpHandler::HandleRunningGames(const httplib::Request& req, httplib::Response& res) {
        auto run_games_info = this->run_game_mgr_->GetRunningGamesAsJson();
        auto resp = WrapBasicInfo(200, "ok", run_games_info);
        res.set_content(resp, "application/json");
    }

    // impl

    std::string HttpHandler::GetInstalledGamesAsJson() {
        auto steam_mgr = context_->GetSteamManager();
        auto games = steam_mgr->GetInstalledGames();
        json obj;
        obj["code"] = 200;
        obj["message"] = "ok";
        obj["steam_installed_dir"] = steam_mgr->GetSteamInstalledPath();
        obj["steam_cache_dir"] = steam_mgr->GetSteamImageCachePath();
        obj["steam_exe_path"] = steam_mgr->GetSteamExePath();
        json game_array = json::array();
        for (const auto& game : games) {
            json game_obj;
            game_obj["app_id"] = game->app_id_;
            game_obj["name"] = game->name_;
            game_obj["installed_dir"] = game->installed_dir_;
            // exes
            {
                auto array = json::array();
                for (auto &exe: game->exes_) {
                    array.push_back(exe);
                }
                game_obj["exes"] = array;
            }

            // exe names
            {
                auto array = json::array();
                for (auto& n : game->exe_names_) {
                    array.push_back(n);
                }
                game_obj["exe_names"] = array;
            }

            // is running
            game_obj["is_running"] = game->is_running_;

            // is installed
            game_obj["is_installed"] = game->is_installed_;

            // steam url
            game_obj["steam_url"] = game->steam_url_;

            // cover url
            game_obj["cover_url"] = game->cover_url_;

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
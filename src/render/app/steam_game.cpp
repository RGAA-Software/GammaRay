//
// Created by RGAA on 2024/4/17.
//

#include "steam_game.h"
#include "tc_common_new/http_client.h"
#include "tc_common_new/log.h"
#include "tc_3rdparty/json/json.hpp"
#include "tc_steam_manager_new/steam_entities.h"

using namespace nlohmann;

namespace tc
{

    SteamGame::SteamGame(const std::shared_ptr<RdContext>& ctx) {
        context_ = ctx;
    }

    std::string SteamGame::GetSteamInstalledPath() {
        return steam_installed_path_;
    }

    std::string SteamGame::GetSteamExePath() {
        return steam_exe_path_;
    }

    std::string SteamGame::GetSteamCachePath() {
        return steam_cache_dir_;
    }

    std::vector<std::shared_ptr<SteamApp>> SteamGame::GetInstalledGames() {
        return games_;
    }

    bool SteamGame::Ready() {
        return !steam_installed_path_.empty() && !steam_exe_path_.empty();
    }

    void SteamGame::RequestSteamGames() {
        auto client = HttpClient::Make("127.0.0.1:20368", "/v1/apps");
        auto resp = client->Request();
        if (resp.status != 200 || resp.body.empty()) {
            LOGE("Request for steam games failed.");
            return;
        }

        games_.clear();

        try {
            auto resp_obj = json::parse(resp.body);
            steam_installed_path_ = resp_obj["steam_installed_dir"].get<std::string>();
            steam_exe_path_ = resp_obj["steam_exe_path"].get<std::string>();
            steam_cache_dir_ = resp_obj["steam_cache_dir"].get<std::string>();

            auto data_obj = resp_obj["data"];
            for (auto& obj : data_obj) {
                auto game = std::make_shared<SteamApp>();
                // app id
                game->app_id_ = obj["app_id"].get<int>();
                // cover name
                game->cover_name_ = obj["cover_name"].get<std::string>();
                // cover url
                game->cover_url_ = obj["cover_url"].get<std::string>();
                // engine
                game->engine_type_ = obj["engine"].get<std::string>();
                // exe names
                for (auto &item: obj["exe_names"]) {
                    game->exe_names_.push_back(item.get<std::string>());
                }
                // exes
                for (auto &item: obj["exes"]) {
                    game->exes_.push_back(item.get<std::string>());
                }

                // installed_dir
                game->installed_dir_ = obj["installed_dir"].get<std::string>();
                // is installed
                game->is_installed_ = obj["is_installed"].get<bool>();
                // is running
                game->is_running_ = obj["is_running"].get<bool>();
                // name
                game->name_ = obj["name"].get<std::string>();
                // steam url
                game->steam_url_ = obj["steam_url"].get<std::string>();

                games_.push_back(game);
            }
        } catch(std::exception& e) {
            LOGE("parse steam games failed: {}", e.what());
            return;
        }

        LOGI("steam_installed_path_: {}", steam_installed_path_);
        LOGI("steam_exe_path_: {}", steam_exe_path_);
        LOGI("steam_cache_dir_: {}", steam_cache_dir_);
        LOGI("game size: {}", games_.size());

    }

}

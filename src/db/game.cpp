//
// Created by hy on 2024/4/10.
//

#include "game.h"
#include "tc_common_new/string_ext.h"
#include <algorithm>

namespace tc
{

    void Game::AssignFrom(const std::shared_ptr<Game>& game) {
        this->game_name_ = game->game_name_;
        this->game_installed_dir_ = game->game_installed_dir_;
        this->cover_url_ = game->cover_url_;
    }

    void Game::CopyFrom(const std::shared_ptr<SteamApp>& steam) {
        this->game_id_ = steam->app_id_;
        this->game_name_ = steam->name_;
        this->game_installed_dir_ = steam->installed_dir_;
        for (auto& e : steam->exes_) {
            this->game_exes_ = this->game_exes_.append(e).append(";");
        }
        for (auto& n : steam->exe_names_) {
            this->game_exe_names_ = this->game_exe_names_.append(n).append(";");
        }
        this->is_installed_ = steam->is_installed_;
        this->steam_url_ = steam->steam_url_;
        this->cover_name_ = steam->cover_name_;
        this->engine_type_ = steam->engine_type_;
        this->cover_url_ = steam->cover_url_;
    }

    void Game::UnpackExePaths() {
        if (!this->game_exes_.empty()) {
            StringExt::Split(this->game_exes_, this->exes_, ";");
            std::erase_if(this->exes_, [](const std::string &item) -> bool {
                return item.empty();
            });
        }
        if (!this->game_exe_names_.empty()) {
            StringExt::Split(this->game_exe_names_, this->exe_names_, ";");
            std::erase_if(this->exe_names_, [](const std::string& item) -> bool {
                return item.empty();
            });
        }
    }

    std::shared_ptr<Game> Game::AsPtr() const {
        auto game = std::make_shared<Game>();
        game->id_ = this->id_;
        game->game_id_ = this->game_id_;
        game->game_name_ = this->game_name_;
        game->game_installed_dir_ = this->game_installed_dir_;
        game->game_exes_ = this->game_exes_;
        game->game_exe_names_ = this->game_exe_names_;
        game->is_installed_ = this->is_installed_;
        game->steam_url_ = this->steam_url_;
        game->cover_name_ = this->cover_name_;
        game->engine_type_ = this->engine_type_;
        game->cover_url_ = this->cover_url_;
        // not in database
//        game->cover_pixmap_ = this->cover_pixmap_;
//        game->exes_ = this->exes_;
//        game->exe_names_ = this->exe_names_;
        return game;
    }

}
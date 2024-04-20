//
// Created by RGAA on 2024/4/10.
//

#include "game.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/log.h"
#include <algorithm>

namespace tc
{

    void TcGame::AssignFrom(const std::shared_ptr<TcGame>& game) {
        this->game_name_ = game->game_name_;
        this->game_installed_dir_ = game->game_installed_dir_;
        this->cover_url_ = game->cover_url_;
        this->engine_type_ = game->engine_type_;

        for (auto& e : game->exes_) {
            this->game_exes_.append(StringExt::CopyStr(e)).append(";");
        }
        std::stringstream ss;
        for (auto& n : game->exe_names_) {
            ss << StringExt::CopyStr(n) << ";";
        }
        this->game_exe_names_ = ss.str();
    }

    void TcGame::CopyFrom(const std::shared_ptr<SteamApp>& steam) {
        this->game_id_ = steam->app_id_;
        this->game_name_ = StringExt::CopyStr(steam->name_);
        LOGI("origin name: {}, copy name: {}", steam->name_, this->game_name_);
        this->game_installed_dir_ = StringExt::CopyStr(steam->installed_dir_);
        for (auto& e : steam->exes_) {
            this->game_exes_.append(StringExt::CopyStr(e)).append(";");
        }

        std::stringstream ss;
        for (auto& n : steam->exe_names_) {
            ss << StringExt::CopyStr(n) << ";";
            LOGI("n: {}, copy n: {}", n, StringExt::CopyStr(n));
        }
        this->game_exe_names_ = ss.str();
        LOGI("game_exe_names: {}", this->game_exe_names_);

        this->is_installed_ = steam->is_installed_;
        this->steam_url_ = StringExt::CopyStr(steam->steam_url_);
        this->cover_name_ = StringExt::CopyStr(steam->cover_name_);
        this->engine_type_ = StringExt::CopyStr(steam->engine_type_);
        this->cover_url_ = StringExt::CopyStr(steam->cover_url_);
    }

    void TcGame::UnpackExePaths() {
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

    std::shared_ptr<TcGame> TcGame::AsPtr() const {
        auto game = std::make_shared<TcGame>();
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
        game->cover_pixmap_ = this->cover_pixmap_;
        game->exes_ = this->exes_;
        game->exe_names_ = this->exe_names_;
        return game;
    }

}
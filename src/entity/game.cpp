#include "game.h"

#include <sstream>
#include <format>

namespace tc
{

    Game Game::Make(const QString& name,
                     int app_id,
                     int installed,
                     int running,
                     int update) {
        Game game;
        game.name_ = name;
        game.app_id_ = app_id;
        game.installed_ = installed;
        game.running_ = running;
        game.updating_ = update;
        return game;
    }

    Game Game::Make(int app_id,
                    const QString& name,
                    const QString& installed_path) {
        Game game;
        game.app_id_ = app_id;
        game.name_ = name;
        game.installed_path_ = installed_path;
        //steam://rungameid/518790
        game.steam_url_ = std::format("steam://rungameid/{}", app_id).c_str();
        return game;
    }

    std::string Game::Dump() {
        std::stringstream ss;
        ss << "name: " << name_.toStdString() << ", app id: " << app_id_ << ", installed path: " << installed_path_.toStdString() + ", url: " << steam_url_.toStdString();
        return ss.str();
    }

}
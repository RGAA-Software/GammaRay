#include "game.h"

#include <sstream>

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

    std::string Game::Dump() {
        std::stringstream ss;
        ss << "name: " << name_.toStdString() << ", app id: " << app_id_ << ", installed: " << installed_;
        return ss.str();
    }

}
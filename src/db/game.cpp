//
// Created by hy on 2024/4/10.
//

#include "game.h"

namespace tc
{

    void Game::AssignFrom(const Game& game) {
        this->game_name_ = game.game_name_;
        this->game_path_ = game.game_path_;
        this->cover_path_ = game.cover_path_;
    }

}
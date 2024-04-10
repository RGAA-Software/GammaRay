//
// Created by hy on 2024/4/10.
//

#ifndef TC_SERVER_STEAM_GAME_H
#define TC_SERVER_STEAM_GAME_H

#include <string>

namespace tc
{

    class Game {
    public:
        int id_;
        uint64_t game_id_;
        std::string game_name_;
        std::string game_path_;
        std::string cover_path_;

    public:
        void AssignFrom(const Game& game);
    };

}

#endif //TC_SERVER_STEAM_GAME_H

//
// Created by hy on 2024/4/10.
//

#ifndef TC_SERVER_STEAM_GAME_MANAGER_H
#define TC_SERVER_STEAM_GAME_MANAGER_H

#include <any>
#include <memory>

#include <sqlite_orm/sqlite_orm.h>
using namespace sqlite_orm;

#include "game.h"

namespace tc
{
    class GrContext;

    class GameManager {
    public:
        explicit GameManager(const std::shared_ptr<GrContext>& ctx);
        void Init();

        void SaveOrUpdateGame(const std::shared_ptr<TcGame>& game);
        std::shared_ptr<TcGame> GetGameByGameId(uint64_t gid);
        std::vector<std::shared_ptr<TcGame>> GetAllGames();
        void DeleteGameByGameId(uint64_t gid);
        void BatchSaveOrUpdateGames(const std::vector<std::shared_ptr<TcGame>>& games);

    private:
        auto GetStorageTypeValue();
        auto InitAppDatabase(const std::string& name);

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::any db_storage_;
    };
}

#endif //TC_SERVER_STEAM_GAME_MANAGER_H

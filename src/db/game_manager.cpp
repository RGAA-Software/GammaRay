//
// Created by hy on 2024/4/10.
//

#include "game_manager.h"

namespace tc
{
    GameManager::GameManager(const std::shared_ptr<Context>& ctx) {
        context_ = ctx;
    }

    auto GameManager::InitAppDatabase(const std::string& name) {
        auto st = make_storage(name,
            make_table("preview_position",
                make_column("id", &Game::id_, primary_key()),
                make_column("game_id", &Game::game_id_),
                make_column("game_name", &Game::game_name_),
                make_column("game_path", &Game::game_path_),
                make_column("cover_path", &Game::cover_path_))
        );
        return st;
    }

    auto GameManager::GetStorageTypeValue() {
        return InitAppDatabase("");
    }

    void GameManager::Init() {
        auto storage = InitAppDatabase("game.db");
        db_storage_ = storage;
        storage.sync_schema();
        auto type = typeid(storage).name();
    }

    void GameManager::SaveOrUpdateGame(const Game& game) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);

        auto games = storage.get_all<Game>(where(c(&Game::game_id_) == game.game_id_));
        if (!games.empty() && game.game_id_ > 0) {
            for (auto& g : games) {
                // update fields.
                g.AssignFrom(game);
                storage.update(g);
            }
        }
        else {
            storage.insert(game);
        }
    }

    Game GameManager::GetGameByGameId(uint64_t gid) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto games = storage.get_all<Game>(where(c(&Game::game_id_) == gid));
        if (games.empty()) {
            return Game{};
        }
        return games.at(0);
    }

    std::vector<Game> GameManager::GetAllGames() {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto games = storage.get_all<Game>();
        return games;
    }

    void GameManager::DeleteGameByGameId(uint64_t gid) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        // using remove will crash
        storage.remove_all<Game>(where(c(&Game::game_id_) == gid));
    }
}
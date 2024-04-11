//
// Created by hy on 2024/4/10.
//

#include "game_manager.h"
#include "tc_common_new/log.h"

namespace tc
{
    GameManager::GameManager(const std::shared_ptr<Context>& ctx) {
        context_ = ctx;
    }

    auto GameManager::InitAppDatabase(const std::string& name) {
        auto st = make_storage(name,
    make_table("games",
                make_column("id", &Game::id_, primary_key().autoincrement()),
                make_column("game_id", &Game::game_id_),
                make_column("game_name", &Game::game_name_),
                make_column("game_installed_dir", &Game::game_installed_dir_),
                make_column("game_exes", &Game::game_exes_),
                make_column("game_exe_names", &Game::game_exe_names_),
                make_column("is_installed", &Game::is_installed_),
                make_column("steam_url", &Game::steam_url_),
                make_column("cover_name", &Game::cover_name_),
                make_column("engine_type", &Game::engine_type_),
                make_column("cover_url", &Game::cover_url_)
            )
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

    void GameManager::SaveOrUpdateGame(const GamePtr& game) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);

        std::vector<Game> games;
        try {
            games = storage.get_all<Game>(where(c(&Game::game_id_) == game->game_id_));
        } catch(std::exception& e) {
            LOGE("SaveOrUpdate failed: {}", e.what());
        }

        if (!games.empty() && game->game_id_ > 0) {
            for (auto &g: games) {
                // update fields.
                g.AssignFrom(game);
                storage.update(g);
            }
        } else {
            storage.insert(*game);
        }
    }

    GamePtr GameManager::GetGameByGameId(uint64_t gid) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto games = storage.get_all<Game>(where(c(&Game::game_id_) == gid));
        if (games.empty()) {
            return nullptr;
        }
        auto g = games.at(0);
        g.UnpackExePaths();
        return g.AsPtr();
    }

    std::vector<GamePtr> GameManager::GetAllGames() {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        std::vector<GamePtr> target_games;
        try {
            auto games = storage.get_all<Game>();
            for (auto &g: games) {
                g.UnpackExePaths();
                target_games.push_back(g.AsPtr());
            }
        } catch(std::exception& e) {
            LOGE("GetAll failed: {}", e.what());
        }
        return target_games;
    }

    void GameManager::DeleteGameByGameId(uint64_t gid) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        // using remove will crash
        try {
            storage.remove_all<Game>(where(c(&Game::game_id_) == gid));
        } catch(std::exception& e) {
            LOGE("Remove failed: {}, e: {}", gid, e.what());
        }
    }

    void GameManager::BatchSaveOrUpdateGames(const std::vector<GamePtr>& games) {
        for (auto& game : games) {
            SaveOrUpdateGame(game);
        }
    }
}
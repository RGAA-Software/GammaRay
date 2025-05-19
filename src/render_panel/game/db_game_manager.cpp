//
// Created by RGAA on 2024/4/10.
//

#include "db_game_manager.h"
#include "tc_common_new/log.h"
#include <QApplication>

namespace tc
{
    DBGameManager::DBGameManager(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
    }

    auto DBGameManager::InitAppDatabase(const std::string& name) {
        auto st = make_storage(name,
            make_table("games",
                make_column("id", &TcDBGame::id_, primary_key().autoincrement()),
                make_column("game_id", &TcDBGame::game_id_),
                make_column("game_name", &TcDBGame::game_name_),
                make_column("game_installed_dir", &TcDBGame::game_installed_dir_),
                make_column("game_exes", &TcDBGame::game_exes_),
                make_column("game_exe_names", &TcDBGame::game_exe_names_),
                make_column("is_installed", &TcDBGame::is_installed_),
                make_column("steam_url", &TcDBGame::steam_url_),
                make_column("cover_name", &TcDBGame::cover_name_),
                make_column("engine_type", &TcDBGame::engine_type_),
                make_column("cover_url", &TcDBGame::cover_url_)
            )
        );
        return st;
    }

    auto DBGameManager::GetStorageTypeValue() {
        return InitAppDatabase("");
    }

    void DBGameManager::Init() {
        auto db_path = qApp->applicationDirPath() + "/gr_data/gr_game.db";
        auto storage = InitAppDatabase(db_path.toStdString());
        db_storage_ = storage;
        storage.sync_schema();
    }

    void DBGameManager::SaveOrUpdateGame(const TcDBGamePtr& game) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto games = storage.get_all<TcDBGame>(where(c(&TcDBGame::game_id_) == game->game_id_));
        if (!games.empty() && game->game_id_ > 0) {
            for (auto& g : games) {
                g.AssignFrom(game);
                storage.update(g);
            }
        } else {
            storage.insert(*game);
        }
    }

    TcDBGamePtr DBGameManager::GetGameByGameId(uint64_t gid) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto games = storage.get_all<TcDBGame>(where(c(&TcDBGame::game_id_) == gid));
        if (games.empty()) {
            return nullptr;
        }
        auto g = games.at(0);
        g.UnpackExePaths();
        return g.AsPtr();
    }

    std::shared_ptr<TcDBGame> DBGameManager::GetGameByExePath(const std::string& path) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto games = storage.get_all<TcDBGame>(where(c(&TcDBGame::game_exes_) == path));
        if (games.empty()) {
            return nullptr;
        }
        auto g = games.at(0);
        g.UnpackExePaths();
        return g.AsPtr();
    }

    std::vector<TcDBGamePtr> DBGameManager::GetAllGames() {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        std::vector<TcDBGamePtr> target_games;
        try {
            auto games = storage.get_all<TcDBGame>();
            for (auto &g: games) {
                g.UnpackExePaths();
                target_games.push_back(g.AsPtr());
            }
        } catch(std::exception& e) {
            LOGE("GetAll failed: {}", e.what());
        }
        return target_games;
    }

    void DBGameManager::DeleteGameByGameId(uint64_t gid) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        // using remove will crash
        try {
            storage.remove_all<TcDBGame>(where(c(&TcDBGame::game_id_) == gid));
        } catch(std::exception& e) {
            LOGE("Remove failed: {}, e: {}", gid, e.what());
        }
    }

    void DBGameManager::BatchSaveOrUpdateGames(const std::vector<TcDBGamePtr>& games) {
        for (auto& game : games) {
            SaveOrUpdateGame(game);
        }
    }
}
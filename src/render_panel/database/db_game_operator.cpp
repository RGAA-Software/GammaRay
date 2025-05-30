//
// Created by RGAA on 2024/4/10.
//

#include "db_game_operator.h"
#include "tc_common_new/log.h"
#include "gr_database.h"
#include <QApplication>

namespace tc
{
    DBGameOperator::DBGameOperator(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<GrDatabase>& db) {
        context_ = ctx;
        db_ = db;
    }

//    auto DBGameOperator::InitAppDatabase(const std::string& name) {
//        auto st = make_storage(name,
//            make_table("games",
//                make_column("id", &TcDBGame::id_, primary_key().autoincrement()),
//                make_column("game_id", &TcDBGame::game_id_),
//                make_column("game_name", &TcDBGame::game_name_),
//                make_column("game_installed_dir", &TcDBGame::game_installed_dir_),
//                make_column("game_exes", &TcDBGame::game_exes_),
//                make_column("game_exe_names", &TcDBGame::game_exe_names_),
//                make_column("is_installed", &TcDBGame::is_installed_),
//                make_column("steam_url", &TcDBGame::steam_url_),
//                make_column("cover_name", &TcDBGame::cover_name_),
//                make_column("engine_type", &TcDBGame::engine_type_),
//                make_column("cover_url", &TcDBGame::cover_url_)
//            )
//        );
//        return st;
//    }

//    auto DBGameOperator::GetStorageTypeValue() {
//        return InitAppDatabase("");
//    }

//    void DBGameOperator::Init() {
//        auto db_path = qApp->applicationDirPath() + "/gr_data/gr_game.db";
//        auto storage = InitAppDatabase(db_path.toStdString());
//        db_->GetDbStorage() = storage;
//        storage.sync_schema();
//    }

    void DBGameOperator::SaveOrUpdateGame(const TcDBGamePtr& game) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
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

    TcDBGamePtr DBGameOperator::GetGameByGameId(uint64_t gid) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto games = storage.get_all<TcDBGame>(where(c(&TcDBGame::game_id_) == gid));
        if (games.empty()) {
            return nullptr;
        }
        auto g = games.at(0);
        g.UnpackExePaths();
        return g.AsPtr();
    }

    std::shared_ptr<TcDBGame> DBGameOperator::GetGameByExePath(const std::string& path) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto games = storage.get_all<TcDBGame>(where(c(&TcDBGame::game_exes_) == path));
        if (games.empty()) {
            return nullptr;
        }
        auto g = games.at(0);
        g.UnpackExePaths();
        return g.AsPtr();
    }

    std::vector<TcDBGamePtr> DBGameOperator::GetAllGames() {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
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

    void DBGameOperator::DeleteGameByGameId(uint64_t gid) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        // using remove will crash
        try {
            storage.remove_all<TcDBGame>(where(c(&TcDBGame::game_id_) == gid));
        } catch(std::exception& e) {
            LOGE("Remove failed: {}, e: {}", gid, e.what());
        }
    }

    void DBGameOperator::BatchSaveOrUpdateGames(const std::vector<TcDBGamePtr>& games) {
        for (auto& game : games) {
            SaveOrUpdateGame(game);
        }
    }
}
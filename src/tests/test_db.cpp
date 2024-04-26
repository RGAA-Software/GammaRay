//
// Created by RGAA on 2023/12/29.
//

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <map>
#include <iostream>

#include "db/db_game.h"
#include "db/db_game_manager.h"
#include "context.h"

using namespace tc;

int main(int argc, char** argv) {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

TEST(Test_db, read_write) {
    std::cout << "start..." << std::endl;

    auto ctx = std::make_shared<Context>();
    auto gm = std::make_shared<DBGameManager>(ctx);
    gm->Init();
    {
        {
            auto g = std::make_shared<Game>();
            g->game_id_ = 1023;
            g->game_name_ = "test1.9";
            g->game_installed_dir_ = "test_path";
            g->cover_url_ = "cover_path";
            gm->SaveOrUpdateGame(g);
        }
        {
            auto g = std::make_shared<Game>();
            g->game_id_ = 10235656;
            g->game_name_ = "test1dfsadf.9";
            g->game_installed_dir_ = "tesdfadst_path";
            g->cover_url_ = "cover_path";

            gm->SaveOrUpdateGame(g);
        }
    }

    auto fn_query_all = [=]() {
        auto games = gm->GetAllGames();
        assert(!games.empty());

        for (auto& game : games) {
            //std::cout << "id: " << game->id_ << ", game id: " << game->game_id_ << ", name: " << game->game_name_ << std::endl;
        }
    };

    fn_query_all();

    {
        gm->DeleteGameByGameId(1023);
    }
    std::cout << "after delete 1023" << std::endl;
    fn_query_all();

    std::cout << "will update 1025" << std::endl;
    {
        auto g = std::make_shared<Game>();
        g->game_id_ = 10235656;
        g->game_name_ = "tesd=========";
        g->game_installed_dir_ = "tesdfadst_path";
        g->cover_url_ = "cover_path";
        gm->SaveOrUpdateGame(g);
    }
    fn_query_all();

}
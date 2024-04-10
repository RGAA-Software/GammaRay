//
// Created by hy on 2023/12/29.
//

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <map>
#include <iostream>

#include "db/game.h"
#include "db/game_manager.h"
#include "context.h"

using namespace tc;

int main(int argc, char** argv) {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

TEST(Test_db, read_write) {
    std::cout << "start..." << std::endl;

    auto ctx = std::make_shared<Context>();
    auto gm = std::make_shared<GameManager>(ctx);
    gm->Init();
    {
        gm->SaveOrUpdateGame(Game{
            .game_id_ = 1023,
            .game_name_ = "test1.9",
            .game_path_ = "test_path",
            .cover_path_ = "cover_path"
        });

        gm->SaveOrUpdateGame(Game{
            .game_id_ = 1025,
            .game_name_ = "test2",
            .game_path_ = "xxtest_path",
            .cover_path_ = "xxcover_path"
        });
    }

    auto fn_query_all = [=]() {
        auto games = gm->GetAllGames();
        assert(!games.empty());

        for (auto& game : games) {
            std::cout << "id: " << game.id_ << ", game id: " << game.game_id_ << ", name: " << game.game_name_ << std::endl;
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
        gm->SaveOrUpdateGame(Game{
            .game_id_ = 1025,
            .game_name_ = "GGGst2",
            .game_path_ = "NNNt_path",
            .cover_path_ = "zzzz"
        });
    }
    fn_query_all();

}
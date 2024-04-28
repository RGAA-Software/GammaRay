//
// Created by hy on 2024/4/26.
//

#ifndef GAMMARAY_RUN_GAME_MANAGER_H
#define GAMMARAY_RUN_GAME_MANAGER_H

#include <memory>
#include <string>
#include <vector>

#include "tc_common_new/response.h"

namespace tc
{

    class GrContext;
    class SteamManager;
    class DBGameManager;
    class TcDBGame;
    class SteamApp;

    class RunningGame {
    public:
        std::shared_ptr<TcDBGame> game_;
        std::vector<uint32_t> pids_;
    };

    class RunGameManager {
    public:

        explicit RunGameManager(const std::shared_ptr<GrContext>& ctx);
        ~RunGameManager();

        // 1. steam url: steam://xxxx
        // 2. specific exe path: c:/xx/xx.exe
        Response<bool, std::string> StartGame(const std::string& game_path, const std::vector<std::string>& args);
        // see above
        Response<bool, std::string> StopGame(const std::string& game_path);

        void CheckRunningGame();

        [[nodiscard]] std::vector<std::shared_ptr<RunningGame>> GetRunningGames() const { return running_games_; }
        std::string GetRunningGamesAsJson();
        std::string GetRunningGamesAsProto();
        std::vector<uint32_t> GetRunningGameIds();

    private:
        std::shared_ptr<SteamApp> FindInSteamManager(const std::string& game_path);
        std::shared_ptr<TcDBGame> FindInDBGameManager(const std::string& game_path);
        bool StartSteamGame(const std::string& game_path, const std::vector<std::string>& args);
        Response<bool, std::string> StartNormalGame(const std::string& game_path, const std::vector<std::string>& args);

    private:
        std::shared_ptr<GrContext> gr_ctx_ = nullptr;
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;
        std::shared_ptr<DBGameManager> db_game_manager_ = nullptr;
        std::vector<std::shared_ptr<RunningGame>> running_games_;
    };

}

#endif //GAMMARAY_RUN_GAME_MANAGER_H

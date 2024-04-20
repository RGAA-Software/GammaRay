//
// Created by RGAA on 2024/4/17.
//

#ifndef TC_APPLICATION_STEAM_GAME_H
#define TC_APPLICATION_STEAM_GAME_H

#include <memory>
#include <string>
#include <vector>

namespace tc
{

    class Context;
    class SteamApp;

    class SteamGame {
    public:

        explicit SteamGame(const std::shared_ptr<Context>& ctx);
        std::string GetSteamInstalledPath();
        std::string GetSteamExePath();
        std::string GetSteamCachePath();
        std::vector<std::shared_ptr<SteamApp>> GetInstalledGames();
        bool Ready();
        void RequestSteamGames();

    private:
        std::shared_ptr<Context> context_ = nullptr;
        std::string steam_installed_path_;
        std::string steam_exe_path_;
        std::string steam_cache_dir_;
        std::vector<std::shared_ptr<SteamApp>> games_;
    };

}

#endif //TC_APPLICATION_STEAM_GAME_H

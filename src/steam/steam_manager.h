//
// Created by hy on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_STEAM_MANAGER_H
#define TC_SERVER_STEAM_STEAM_MANAGER_H

#include <memory>
#include <QString>

#include "context.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "entity/game.h"

namespace tc
{

    class Context;

    class SteamManager {
    public:

        static std::shared_ptr<SteamManager> Make(const std::shared_ptr<Context>& ctx);

        explicit SteamManager(const std::shared_ptr<Context>& ctx);
        ~SteamManager();

        bool ScanInstalledGames();
        std::vector<Game> GetInstalledGames();
        void DumpGamesInfo();

    private:
        QString ScanInstalledSteamPath();
        void QueryInstalledApps(HKEY key);

    private:
        std::shared_ptr<Context> context_ = nullptr;
        QString installed_steam_path_;
        std::wstring steam_app_base_path_;
        std::vector<Game> games_;
    };

}
#endif //TC_SERVER_STEAM_STEAM_MANAGER_H

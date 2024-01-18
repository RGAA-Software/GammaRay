//
// Created by hy on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_STEAM_MANAGER_H
#define TC_SERVER_STEAM_STEAM_MANAGER_H

#include <memory>
#include <QString>

#include "context.h"

namespace tc
{

    class Context;

    class SteamManager {
    public:

        static std::shared_ptr<SteamManager> Make(const std::shared_ptr<Context>& ctx);

        SteamManager(const std::shared_ptr<Context>& ctx);
        ~SteamManager();

        bool Init();

    private:
        QString ScanInstalledSteamPath();

    private:
        std::shared_ptr<Context> context_ = nullptr;
        QString installed_steam_path_;
    };

}
#endif //TC_SERVER_STEAM_STEAM_MANAGER_H

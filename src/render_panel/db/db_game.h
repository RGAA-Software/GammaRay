//
// Created by RGAA on 2024/4/10.
//

#ifndef TC_SERVER_STEAM_GAME_H
#define TC_SERVER_STEAM_GAME_H

#include <string>
#include <any>
#include <QDebug>

#include "tc_steam_manager_new/steam_entities.h"
#include "tc_common_new/log.h"

namespace tc
{

    class TcDBGame {
    public:
        int id_{};
        uint64_t game_id_{};
        std::string game_name_{};
        std::string game_installed_dir_{};
        std::string game_exes_{};
        std::string game_exe_names_{};
        int is_installed_{};
        std::string steam_url_{};
        std::string cover_name_{};
        std::string engine_type_{};
        // steam changed cache dir, this field was deprecated
        std::string cover_url_{};

        // not in database
        std::any cover_pixmap_;
        std::vector<std::string> exes_{};
        std::vector<std::string> exe_names_{};
        std::vector<uint32_t> running_pids_;

    public:
        void AssignFrom(const std::shared_ptr<TcDBGame>& game);
        void CopyFrom(const std::shared_ptr<SteamApp>& steam);
        void UnpackExePaths();
        [[nodiscard]] std::shared_ptr<TcDBGame> AsPtr() const;
        bool IsSteamGame() const;

    };

    using TcDBGamePtr = std::shared_ptr<TcDBGame>;

}

#endif //TC_SERVER_STEAM_GAME_H

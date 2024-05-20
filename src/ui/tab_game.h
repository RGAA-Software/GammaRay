//
// Created by RGAA on 2024-04-09.
//

#ifndef TC_SERVER_STEAM_TABGAME_H
#define TC_SERVER_STEAM_TABGAME_H

#include "tab_base.h"
#include <QListWidget>
#include <QListWidgetItem>
#include "db/db_game.h"

namespace tc
{

    class SteamApp;
    class SteamManager;

    class TabGame : public TabBase {
    public:

        explicit TabGame(const std::shared_ptr<GrApplication>& app, QWidget* parent = nullptr);
        ~TabGame() override;

        void OnTabShow() override;
        void OnTabHide() override;
    private:
        void ScanInstalledGames();
        QListWidgetItem* AddItem(int idx, const TcDBGamePtr& game);
        QSize GetItemSize();
        void AddItems(const std::vector<TcDBGamePtr>& games);
        void LoadCover(const TcDBGamePtr& game);

    private:
        QListWidget* list_widget_ = nullptr;
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;
        std::vector<TcDBGamePtr> games_;
    };

}

#endif //TC_SERVER_STEAM_TABGAME_H

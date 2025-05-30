//
// Created by RGAA on 2024-04-09.
//

#ifndef TC_SERVER_STEAM_TABGAME_H
#define TC_SERVER_STEAM_TABGAME_H

#include "tab_base.h"
#include <functional>
#include "render_panel/database/db_game.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QProcess>

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
        QListWidgetItem* AddItem(const TcDBGamePtr& game);
        static QSize GetItemSize();
        void AddItems(const std::vector<TcDBGamePtr>& games);
        void LoadCover(const TcDBGamePtr& game);
        void UpdateRunningStatus(const std::vector<uint64_t>& game_ids);
        void VisitListWidget(std::function<void(QListWidgetItem* item, QWidget* item_widget)>&& cbk);
        void ShowAddGamePanel();
        void RefreshGames();

    private:
        QListWidget* list_widget_ = nullptr;
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;
        std::vector<TcDBGamePtr> games_;
    };

}

#endif //TC_SERVER_STEAM_TABGAME_H

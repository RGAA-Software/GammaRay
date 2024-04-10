//
// Created by RGAA on 2024-04-09.
//

#ifndef TC_SERVER_STEAM_TABGAME_H
#define TC_SERVER_STEAM_TABGAME_H

#include "tab_base.h"
#include <QListWidget>
#include <QListWidgetItem>

namespace tc
{

    class SteamApp;
    class SteamManager;

    class TabGame : public TabBase {
    public:

        explicit TabGame(const std::shared_ptr<Context>& ctx, QWidget* parent = nullptr);
        ~TabGame() override;

        void OnTabShow() override;
        void OnTabHide() override;
    private:
        void ScanInstalledGames();
        QListWidgetItem* AddItem(int idx, const std::shared_ptr<SteamApp>& game);
        QSize GetItemSize();

    private:
        QListWidget* list_widget_ = nullptr;
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;
    };

}

#endif //TC_SERVER_STEAM_TABGAME_H

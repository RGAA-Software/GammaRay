//
// Created by hy on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_WORKSPACE_H
#define TC_SERVER_STEAM_WORKSPACE_H

#include <QMainWindow>
#include <memory>
#include <QPushButton>
#include <QStackedWidget>

namespace tc
{

    enum class TabName {
        kTabServer,
        kTabGames,
        kTabSettings,
    };

    class TabBase;
    class GrApplication;

    class Workspace : public QMainWindow {
    public:
        Workspace();

    private:
        void ChangeTab(const TabName& tn);

    private:
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::map<TabName, TabBase*> tabs_;
        QPushButton* btn_tab_server_ = nullptr;
        QPushButton* btn_tab_games_ = nullptr;
        QPushButton* btn_tab_settings_ = nullptr;
        QStackedWidget* stacked_widget_ = nullptr;
    };

}

#endif //TC_SERVER_STEAM_WORKSPACE_H

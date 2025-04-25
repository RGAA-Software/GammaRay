//
// Created by RGAA on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_WORKSPACE_H
#define TC_SERVER_STEAM_WORKSPACE_H

#include <QMainWindow>
#include <memory>
#include <QPushButton>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include "theme/QtAdvancedStylesheet.h"

namespace tc
{

    struct MainWindowPrivate {
        explicit MainWindowPrivate(QMainWindow* _public) : _this(_public) {}

        QMainWindow* _this;
        acss::QtAdvancedStylesheet* AdvancedStyleSheet{};
        QVector<QPushButton*> ThemeColorButtons;
    };

    enum class TabName {
        kTabServer,
        kTabServerStatus,
        kTabGames,
        kTabSettings,
        kTabProfile,
    };

    class TabBase;
    class GrApplication;
    class GrSettings;
    class MessageListener;

    class GrWorkspace : public QMainWindow, public std::enable_shared_from_this<GrWorkspace> {
    public:
        GrWorkspace();
        void Init();
        void closeEvent(QCloseEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;

    private:
        void ChangeTab(const TabName& tn);
        void InitListeners();

    private:
        std::shared_ptr<GrApplication> app_ = nullptr;
        GrSettings* settings_ = nullptr;
        std::map<TabName, TabBase*> tabs_;
        QPushButton* btn_tab_server_ = nullptr;
        QPushButton* btn_tab_server_status_ = nullptr;
        QPushButton* btn_tab_games_ = nullptr;
        QPushButton* btn_tab_settings_ = nullptr;
        QPushButton* btn_tab_profile_ = nullptr;
        QStackedWidget* stacked_widget_ = nullptr;
        MainWindowPrivate* theme_ = nullptr;
        QSystemTrayIcon* sys_tray_icon_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };

    extern std::shared_ptr<GrWorkspace> grWorkspace;

}

#endif //TC_SERVER_STEAM_WORKSPACE_H

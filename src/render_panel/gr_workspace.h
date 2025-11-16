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
        kTabCoPhone,
        kTabSettings,
        kTabSecurity,
        kTabProfile,
        kTabHWInfo,
    };

    class TabBase;
    class GrApplication;
    class GrSettings;
    class MessageListener;
    class SkinInterface;

    class GrWorkspace : public QMainWindow, public std::enable_shared_from_this<GrWorkspace> {
    public:
        GrWorkspace(bool run_automatically);
        void Init();
        void closeEvent(QCloseEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;

    private:
        void ChangeTab(const TabName& tn);
        void InitListeners();
        void ForceStopAllPrograms(bool uninstall_service);
        void ShowUserRegisterDialog();

    private:
        std::shared_ptr<GrApplication> app_ = nullptr;
        GrSettings* settings_ = nullptr;
        SkinInterface* skin_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::map<TabName, TabBase*> tabs_;
        QPushButton* btn_tab_server_ = nullptr;
        QPushButton* btn_tab_server_status_ = nullptr;
        QPushButton* btn_tab_games_ = nullptr;
        QPushButton* btn_security_ = nullptr;
        QPushButton* btn_tab_settings_ = nullptr;
        QPushButton* btn_tab_cophone_ = nullptr;
        QPushButton* btn_tab_profile_ = nullptr;
        QPushButton* btn_tab_hw_info_ = nullptr;
        QStackedWidget* stacked_widget_ = nullptr;
        MainWindowPrivate* theme_ = nullptr;
        QSystemTrayIcon* sys_tray_icon_ = nullptr;
        QPushButton* btn_exit_ = nullptr;
        QPushButton* btn_uninstall_ = nullptr;
        // is started by OS when logon?
        bool run_automatically_ = false;
    };

    extern std::shared_ptr<GrWorkspace> grWorkspace;

}

#endif //TC_SERVER_STEAM_WORKSPACE_H

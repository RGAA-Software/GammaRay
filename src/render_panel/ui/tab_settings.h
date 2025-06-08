//
// Created by RGAA on 2024-04-09.
//

#ifndef TC_SERVER_STEAM_TABSETTINGS_H
#define TC_SERVER_STEAM_TABSETTINGS_H

#include "tab_base.h"
#include <QStackedWidget>
#include <QPushButton>

namespace tc
{

    enum class StTabName {
        kStGeneral,
        kStNetwork,
        kStSecurity,
        kStPlugins,
        kStController,
        kStAboutMe,
    };

    class TabSettings : public TabBase {
    public:

        explicit TabSettings(const std::shared_ptr<GrApplication>& app, QWidget* parent = nullptr);
        ~TabSettings() override;

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        void ChangeTab(const StTabName& tn);

    private:

        std::map<StTabName, TabBase*> tabs_;
        QStackedWidget* stacked_widget_ = nullptr;
        QPushButton* btn_network_ = nullptr;
        QPushButton* btn_security_ = nullptr;
        QPushButton* btn_input_ = nullptr;
        QPushButton* btn_plugins_ = nullptr;
        QPushButton* btn_controller = nullptr;
        QPushButton* btn_about_me_ = nullptr;

    };

}

#endif //TC_SERVER_STEAM_TABGAME_H

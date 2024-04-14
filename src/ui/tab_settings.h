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
        kStClient,
        kStInput,
        kStMedia,
        kStNetwork,
        kStEncoder,
    };

    class TabSettings : public TabBase {
    public:

        explicit TabSettings(const std::shared_ptr<Context>& ctx, QWidget* parent = nullptr);
        ~TabSettings() override;

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        void ChangeTab(const StTabName& tn);

    private:

        std::map<StTabName, TabBase*> tabs_;
        QStackedWidget* stacked_widget_ = nullptr;
        QPushButton* btn_client_ = nullptr;
        QPushButton* btn_input_ = nullptr;
        QPushButton* btn_media_ = nullptr;
        QPushButton* btn_network_ = nullptr;
        QPushButton* btn_encoder_ = nullptr;

    };

}

#endif //TC_SERVER_STEAM_TABGAME_H

//
// Created by RGAA on 2024-04-09.
//

#ifndef TC_SERVER_STEAM_TABSETTINGS_H
#define TC_SERVER_STEAM_TABSETTINGS_H

#include "tab_base.h"

namespace tc
{

    class TabSettings : public TabBase {
    public:

        explicit TabSettings(const std::shared_ptr<Context>& ctx, QWidget* parent = nullptr);
        ~TabSettings();

        void OnTabShow() override;
        void OnTabHide() override;

    };

}

#endif //TC_SERVER_STEAM_TABGAME_H

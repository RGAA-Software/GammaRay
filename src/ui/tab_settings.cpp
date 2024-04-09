//
// Created by RGAA on 2024-04-09.
//

#include "tab_settings.h"

namespace tc
{

    TabSettings::TabSettings(const std::shared_ptr<Context>& ctx, QWidget* parent) : TabBase(ctx, parent) {
        setStyleSheet("background:#280499;");
    }

    TabSettings::~TabSettings() {

    }

    void TabSettings::OnTabShow() {

    }

    void TabSettings::OnTabHide() {

    }

}
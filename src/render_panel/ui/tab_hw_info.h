//
// Created by RGAA on 22/03/2025.
//

#ifndef GAMMARAY_TAB_HW_INFO_INTERNALS_H
#define GAMMARAY_TAB_HW_INFO_INTERNALS_H

#include <map>
#include <QStackedWidget>
#include <QPushButton>
#include "tab_base.h"

namespace tc
{

    class CustomTabBtn;

    class TabHWInfo : public TabBase {
    public:
        TabHWInfo(const std::shared_ptr<GrApplication>& app, QWidget *parent);

        void OnTabShow() override;
        void OnTabHide() override;

    private:

    };

}

#endif //GAMMARAY_TAB_PROFILE_H

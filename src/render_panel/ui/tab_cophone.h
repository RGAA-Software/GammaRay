//
// Created by RGAA on 22/03/2025.
//

#ifndef GAMMARAY_TAB_COPHONE_H
#define GAMMARAY_TAB_COPHONE_H

#include "tab_base.h"
#include <QListWidget>
#include <QStackedWidget>

namespace tc
{

    class NoMarginHLayout;
    class NoMarginVLayout;
    class AccountSdk;
    class AccountDevice;
    class AccountProfile;

    class TabCoPhone : public TabBase {
    public:
        TabCoPhone(const std::shared_ptr<GrApplication>& app, QWidget *parent);

        void OnTabShow() override;
        void OnTabHide() override;

        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;
        void dropEvent(QDropEvent *event) override;

    private:

    private:
        NoMarginHLayout* root_layout_ = nullptr;

    };

}

#endif //GAMMARAY_TAB_PROFILE_H

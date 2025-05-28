//
// Created by RGAA on 22/03/2025.
//

#ifndef GAMMARAY_TAB_PROFILE_H
#define GAMMARAY_TAB_PROFILE_H

#include "tab_base.h"

namespace tc
{

    class TabProfile : public TabBase {
    public:
        TabProfile(const std::shared_ptr<GrApplication>& app, QWidget *parent);

        void OnTabShow() override;
        void OnTabHide() override;

        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;
        void dropEvent(QDropEvent *event) override;

    };

}

#endif //GAMMARAY_TAB_PROFILE_H

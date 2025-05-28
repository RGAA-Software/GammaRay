//
// Created by RGAA on 22/03/2025.
//

#ifndef GAMMARAY_TAB_SECURITY_INTERNALS_H
#define GAMMARAY_TAB_SECURITY_INTERNALS_H

#include <map>
#include <QStackedWidget>
#include <QPushButton>
#include "tab_base.h"

namespace tc
{

    enum class StSecurityTabName {
        kStVisitor,
        kStFileTransfer,
    };

    class CustomTabBtn;

    class TabSecurityInternals : public TabBase {
    public:
        TabSecurityInternals(const std::shared_ptr<GrApplication>& app, QWidget *parent);

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        void ChangeTab(const StSecurityTabName& tn);

    private:
        std::map<StSecurityTabName, TabBase*> tabs_;
        QStackedWidget* stacked_widget_ = nullptr;
        CustomTabBtn* btn_visitor_ = nullptr;
        CustomTabBtn* btn_file_transfer_ = nullptr;
    };

}

#endif //GAMMARAY_TAB_PROFILE_H

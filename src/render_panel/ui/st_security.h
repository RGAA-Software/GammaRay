//
// Created by RGAA on 2024-06-10.
//

#ifndef GAMMARAY_ST_SECURITY_H
#define GAMMARAY_ST_SECURITY_H

#include <QLabel>
#include "tab_base.h"

namespace tc
{
    class GrApplication;

    class StSecurity : public TabBase {
    public:
        explicit StSecurity(const std::shared_ptr<GrApplication>& app, QWidget* parent = nullptr);
        ~StSecurity() override = default;

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        QLabel* license_ = nullptr;
    };

}

#endif //GAMMARAY_ST_ABOUT_ME_H

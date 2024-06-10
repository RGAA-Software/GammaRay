//
// Created by RGAA on 2024-06-10.
//

#ifndef GAMMARAY_ST_ABOUT_ME_H
#define GAMMARAY_ST_ABOUT_ME_H

#include "tab_base.h"

namespace tc
{
    class GrApplication;

    class StAboutMe : public TabBase {
    public:
        explicit StAboutMe(const std::shared_ptr<GrApplication>& app, QWidget* parent = nullptr);
        ~StAboutMe() override = default;

        void OnTabShow() override;
        void OnTabHide() override;
    };

}

#endif //GAMMARAY_ST_ABOUT_ME_H

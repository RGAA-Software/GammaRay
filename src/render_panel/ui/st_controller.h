//
// Created by RGAA on 2024-06-10.
//

#ifndef GAMMARAY_ST_CONTROLLER_H
#define GAMMARAY_ST_CONTROLLER_H

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "tab_base.h"

namespace tc
{
    class GrApplication;

    class StController : public TabBase {
    public:
        explicit StController(const std::shared_ptr<GrApplication>& app, QWidget* parent = nullptr);
        ~StController() override = default;

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        QLineEdit* et_screen_recording_path_ = nullptr;
        QPushButton* btn_select_screen_recording_path_ = nullptr;
    };

}

#endif //GAMMARAY_ST_ABOUT_ME_H

//
// Created by RGAA on 2024-04-11.
//

#include "st_media.h"

namespace tc
{

    StMedia::StMedia(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        setStyleSheet("background: #009090;");
    }

    void StMedia::OnTabShow() {

    }

    void StMedia::OnTabHide() {

    }

}
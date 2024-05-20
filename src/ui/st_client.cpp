//
// Created by RGAA on 2024-04-11.
//

#include "st_client.h"

namespace tc
{

    StClient::StClient(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        setStyleSheet("background: #9cc090;");
    }

    void StClient::OnTabShow() {

    }

    void StClient::OnTabHide() {

    }

}
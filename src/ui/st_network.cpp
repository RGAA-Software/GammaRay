//
// Created by RGAA on 2024-04-11.
//

#include "st_network.h"

namespace tc
{

    StNetwork::StNetwork(const std::shared_ptr<GrContext>& ctx, QWidget *parent) : TabBase(ctx, parent) {
        setStyleSheet("background: #9ff090;");
    }

    void StNetwork::OnTabShow() {

    }

    void StNetwork::OnTabHide() {

    }

}
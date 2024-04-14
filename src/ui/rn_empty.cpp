//
// Created by RGAA on 2024-04-11.
//

#include "rn_empty.h"

namespace tc
{

    RnEmpty::RnEmpty(const std::shared_ptr<Context>& ctx, QWidget *parent) : TabBase(ctx, parent) {
        setStyleSheet("background: #909090;");
    }

    void RnEmpty::OnTabShow() {

    }

    void RnEmpty::OnTabHide() {

    }

}
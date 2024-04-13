//
// Created by hy on 2024/4/9.
//

#include "tab_base.h"
#include "settings.h"

namespace tc
{
    TabBase::TabBase(const std::shared_ptr<Context>& ctx, QWidget* parent) : QWidget(parent) {
        context_ = ctx;
        settings_ = Settings::Instance();
    }

    TabBase::~TabBase() {

    }

    void TabBase::OnTabShow() {

    }

    void TabBase::OnTabHide() {

    }
}
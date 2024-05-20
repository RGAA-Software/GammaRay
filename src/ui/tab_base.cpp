//
// Created by RGAA on 2024/4/9.
//

#include "tab_base.h"
#include "gr_settings.h"
#include "gr_context.h"
#include "gr_application.h"
#include "tc_common_new/message_notifier.h"

namespace tc
{
    TabBase::TabBase(const std::shared_ptr<GrApplication>& app, QWidget* parent) : QWidget(parent) {
        app_ = app;
        context_ = app->GetContext();
        settings_ = GrSettings::Instance();
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
    }

    TabBase::~TabBase() {

    }

    void TabBase::OnTabShow() {

    }

    void TabBase::OnTabHide() {

    }
}
//
// Created by RGAA on 2024/4/9.
//

#include "tab_base.h"
#include "gr_settings.h"
#include "gr_context.h"
#include "tc_common_new/message_notifier.h"

namespace tc
{
    TabBase::TabBase(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : QWidget(parent) {
        context_ = ctx;
        settings_ = GrSettings::Instance();
        msg_listener_ = ctx->GetMessageNotifier()->CreateListener();
    }

    TabBase::~TabBase() {

    }

    void TabBase::OnTabShow() {

    }

    void TabBase::OnTabHide() {

    }
}
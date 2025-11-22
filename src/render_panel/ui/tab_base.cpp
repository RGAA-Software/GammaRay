//
// Created by RGAA on 2024/4/9.
//

#include "tab_base.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_application.h"
#include "tc_common_new/message_notifier.h"
#include "render_panel/gr_statistics.h"
#include "render_panel/gr_app_messages.h"

// 测试崩溃函数
void CrashFunction() {
    int* ptr = nullptr;
    *ptr = 42;  // 人为制造崩溃
}

namespace tc
{
    TabBase::TabBase(const std::shared_ptr<GrApplication>& app, QWidget* parent) : QWidget(parent) {
        app_ = app;
        context_ = app->GetContext();
        settings_ = GrSettings::Instance();
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        statistics_ = GrStatistics::Instance();

        msg_listener_->Listen<MsgLanguageChanged>([=, this](const MsgLanguageChanged& msg) {
            this->OnTranslate();
            //CrashFunction();
        });
    }

    TabBase::~TabBase() {

    }

    void TabBase::OnTabShow() {

    }

    void TabBase::OnTabHide() {

    }

    void TabBase::OnTranslate() {

    }
}
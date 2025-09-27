//
// Created by RGAA on 22/03/2025.
//

#include "tab_hw_info.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include "app_colors.h"
#include "tc_pushbutton.h"
#include "no_margin_layout.h"
#include "tc_common_new/log.h"
#include "tc_qt_widget/custom_tab_btn.h"
#include "hw_info/hw_info_widget.h"
#include "render_panel/gr_context.h"
#include "tc_common_new/message_notifier.h"
#include "render_panel/gr_app_messages.h"

namespace tc
{

    TabHWInfo::TabHWInfo(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginHLayout();
        setLayout(root_layout);

        hw_widget_ = new HWInfoWidget(false, this);
        root_layout->addWidget(hw_widget_);

        msg_listener_->Listen<MsgHWInfo>([=, this](const MsgHWInfo& msg) {
            hw_widget_->OnSysInfoCallback(msg.sys_info_);
        });
    }

    void TabHWInfo::OnTabShow() {

    }

    void TabHWInfo::OnTabHide() {

    }
}
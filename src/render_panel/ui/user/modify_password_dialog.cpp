//
// Created by RGAA on 2023-08-18.
//

#include "modify_password_dialog.h"
#include <QValidator>
#include <QButtonGroup>
#include <QRadioButton>
#include "tc_qt_widget/sized_msg_box.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_qt_widget/tc_password_input.h"
#include "tc_dialog.h"
#include "tc_label.h"
#include "tc_pushbutton.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/gr_application.h"
#include "render_panel/user/gr_user_manager.h"
#include "tc_common_new/log.h"

namespace tc
{

    ModifyPasswordDialog::ModifyPasswordDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        context_ = ctx;
        setFixedSize(375, 330);
        CreateLayout();
    }

    ModifyPasswordDialog::~ModifyPasswordDialog() = default;

    void ModifyPasswordDialog::CreateLayout() {
        setWindowTitle(tcTr("id_edit_password"));

        auto item_width = 320;
        auto edit_size = QSize(item_width, 35);

        auto root_layout = new NoMarginHLayout();
        auto content_layout = new NoMarginVLayout();
        root_layout->addStretch();
        root_layout->addLayout(content_layout);
        root_layout->addStretch();
        root_layout_->addLayout(root_layout);

        content_layout->addSpacing(25);

        // 1. password
        {
            auto layout = new NoMarginVLayout();
            auto label = new TcLabel(this);
            label->setFixedWidth(item_width);
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->SetTextId("id_password");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto edit = new TcPasswordInput(this);
            password_input_ = edit;
            password_input_->SetPassword("");

            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();
            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(25);

        // 2. password again
        {
            auto layout = new NoMarginVLayout();
            auto label = new TcLabel(this);
            label->setFixedWidth(item_width);
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->SetTextId("id_re_password");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto edit = new TcPasswordInput(this);
            password_input_again_ = edit;
            password_input_again_->SetPassword("");

            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();
            content_layout->addLayout(layout);
        }

        content_layout->addStretch();

        // sure button
        {
            auto layout = new NoMarginVLayout();
            auto btn_sure = new TcPushButton();
            btn_sure->SetTextId("id_ok");
            connect(btn_sure, &QPushButton::clicked, this, [=, this] () {
                ModifyPassword();
            });

            layout->addWidget(btn_sure);
            btn_sure->setFixedSize(QSize(item_width, 35));

            content_layout->addLayout(layout);
        }
        content_layout->addSpacing(60);
    }

    void ModifyPasswordDialog::paintEvent(QPaintEvent *event) {
        TcCustomTitleBarDialog::paintEvent(event);
    }

    std::string ModifyPasswordDialog::GetPassword() {
        return password_input_->GetPassword().toStdString();
    }

    std::string ModifyPasswordDialog::GetPasswordAgain() {
        return password_input_again_->GetPassword().toStdString();
    }

    void ModifyPasswordDialog::ModifyPassword() {
        auto user_mgr = grApp->GetUserManager();
        auto password = GetPassword();
        auto password_again = GetPasswordAgain();
        if (password.empty() || password_again.empty()) {
            return;
        }
        if (password != password_again) {
            TcDialog dialog(tcTr("id_error"), tcTr("id_password_invalid"));
            dialog.exec();
            return;
        }
        bool r = user_mgr->ModifyPassword(password);
        if (r) {
            done(kDoneOk);
        }
        else {
            LOGE("ModifyPassword failed: {}", password);
        }
    }

}
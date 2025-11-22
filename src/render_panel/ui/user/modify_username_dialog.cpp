//
// Created by RGAA on 2023-08-18.
//

#include "modify_username_dialog.h"
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

    ModifyUsernameDialog::ModifyUsernameDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        context_ = ctx;
        setFixedSize(375, 330);
        CreateLayout();
    }

    ModifyUsernameDialog::~ModifyUsernameDialog() = default;

    void ModifyUsernameDialog::CreateLayout() {
        setWindowTitle(tcTr("id_edit_username"));

        auto item_width = 320;
        auto edit_size = QSize(item_width, 35);

        auto root_layout = new NoMarginHLayout();
        auto content_layout = new NoMarginVLayout();
        root_layout->addStretch();
        root_layout->addLayout(content_layout);
        root_layout->addStretch();
        root_layout_->addLayout(root_layout);

        content_layout->addSpacing(25);

        // 1. username
        {
            auto layout = new NoMarginVLayout();
            auto label = new TcLabel(this);
            label->setFixedWidth(item_width);
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->SetTextId("id_username");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto edit = new QLineEdit(this);
            edt_username_ = edit;
            edt_username_->setText("");

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
                Login();
            });

            layout->addWidget(btn_sure);
            btn_sure->setFixedSize(QSize(item_width, 35));

            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(60);
    }

    void ModifyUsernameDialog::paintEvent(QPaintEvent *event) {
        TcCustomTitleBarDialog::paintEvent(event);
    }

    std::string ModifyUsernameDialog::GetUsername() {
        return edt_username_->text().toStdString();
    }

    std::string ModifyUsernameDialog::GetPassword() {
        return password_input_->GetPassword().toStdString();
    }

    void ModifyUsernameDialog::Login() {
        auto user_mgr = grApp->GetUserManager();
        auto username = GetUsername();
        auto password = GetPassword();
        if (username.empty() || password.empty()) {
            return;
        }
        bool r = user_mgr->Login(username, password);
        if (r) {
            done(0);
        }
        else {
            LOGE("Login failed: {} {}", username, password);
        }
    }

}
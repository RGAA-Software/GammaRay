//
// Created by RGAA on 2023-08-18.
//

#include "user_login_dialog.h"
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

    UserLoginDialog::UserLoginDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        context_ = ctx;
        setFixedSize(375, 475);
        CreateLayout();
    }

    UserLoginDialog::~UserLoginDialog() = default;

    void UserLoginDialog::CreateLayout() {
        setWindowTitle(tcTr("id_login"));

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

        content_layout->addSpacing(25);

        // 2. password
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

        content_layout->addSpacing(40);

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

            content_layout->addSpacing(105);
            content_layout->addLayout(layout);
        }

        // or
        {
            auto lbl = new TcLabel(this);
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            layout->addWidget(lbl);
            layout->addStretch();
            lbl->setText("-or-");
            content_layout->addSpacing(20);
            content_layout->addLayout(layout);
        }

        {

            auto style = "QLabel {"
                "    background-color: #ffffff;"
                "    color: #2171cf;"
                "    font-size: 13px;"
                "    font-weight: bold;"
                "}"
                "QLabel:hover {"
                "    background-color: #ffffff;"  // 鼠标悬浮时的背景色
                "    color: #2979ff;"             // 鼠标悬浮时的文字颜色
                "}"
                "QLabel:pressed {"
                "    background-color: #ffffff;"  // 鼠标按下时的背景色
                "    color: #2171cf;"             // 鼠标按下时的文字颜色
                "}";

            auto layout = new NoMarginHLayout();
            layout->addStretch();

            {
                auto lbl = new TcLabel(this);
                lbl->SetOnClickListener([=, this](QWidget* w) {
                    done(-1);
                });
                layout->addWidget(lbl);
                lbl->setStyleSheet(style);
                lbl->SetTextId("id_register");
            }
            layout->addSpacing(15);
            {
                auto lbl = new TcLabel(this);
                lbl->SetOnClickListener([=, this](QWidget* w) {
                    TcDialog dialog(tcTr("id_tips"), tcTr("id_consult_admin_to_change_password"));
                    dialog.exec();
                });
                layout->addWidget(lbl);
                lbl->setStyleSheet(style);
                lbl->SetTextId("id_forget_password");
            }
            layout->addStretch();

            content_layout->addSpacing(10);
            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(30);
        root_layout_->addStretch();
    }

    void UserLoginDialog::paintEvent(QPaintEvent *event) {
        TcCustomTitleBarDialog::paintEvent(event);
    }

    std::string UserLoginDialog::GetUsername() {
        return edt_username_->text().toStdString();
    }

    std::string UserLoginDialog::GetPassword() {
        return password_input_->GetPassword().toStdString();
    }

    void UserLoginDialog::Login() {
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
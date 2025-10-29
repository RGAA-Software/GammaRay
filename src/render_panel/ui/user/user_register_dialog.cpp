//
// Created by RGAA on 2023-08-18.
//

#include "user_register_dialog.h"
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

namespace tc
{

    UserRegisterDialog::UserRegisterDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        context_ = ctx;
        setFixedSize(375, 475);
        CreateLayout();
    }

    UserRegisterDialog::~UserRegisterDialog() = default;

    void UserRegisterDialog::CreateLayout() {
        setWindowTitle(tcTr("id_add_device"));

        auto item_width = 320;
        auto edit_size = QSize(item_width, 35);

        auto root_layout = new NoMarginHLayout();
        auto content_layout = new NoMarginVLayout();
        root_layout->addStretch();
        root_layout->addLayout(content_layout);
        root_layout->addStretch();
        root_layout_->addLayout(root_layout);

        content_layout->addSpacing(25);

        // 0. name
        {
            auto layout = new NoMarginVLayout();

            auto label = new TcLabel(this);
            label->setFixedWidth(item_width);
            label->SetTextId("id_device_id");
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto edit = new QLabel(this);
            edit->setText("DeviceID");
            edit->setStyleSheet("font-size: 16px; font-weight: 700; color: #2979ff;");
            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();

            content_layout->addLayout(layout);

        }

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
            password_input_->SetPassword("Password");

            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();
            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(25);

        // 2. name
        {
            auto layout = new NoMarginVLayout();
            auto label = new TcLabel(this);
            label->setFixedWidth(item_width);
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->SetTextId("id_device_name");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto edit = new QLineEdit(this);
            edt_stream_name_ = edit;
            edt_stream_name_->setText("Name");

            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();
            content_layout->addLayout(layout);
        }

        // sure button
        {
            auto layout = new NoMarginVLayout();
            auto btn_sure = new TcPushButton();
            btn_sure->SetTextId("id_ok");
            connect(btn_sure, &QPushButton::clicked, this, [=, this] () {

                this->close();
            });

            layout->addWidget(btn_sure);
            btn_sure->setFixedSize(QSize(item_width, 35));

            content_layout->addSpacing(105);
            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(30);
        root_layout_->addStretch();
    }



    void UserRegisterDialog::paintEvent(QPaintEvent *event) {
        TcCustomTitleBarDialog::paintEvent(event);
    }

}
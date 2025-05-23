//
// Created by RGAA on 2023-08-18.
//

#include "input_remote_pwd_dialog.h"
#include <QValidator>
#include <QButtonGroup>
#include <QRadioButton>
#include <QTextEdit>
#include "tc_dialog.h"
#include "stream_item.h"
#include "client/ct_app_message.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/util/conn_info_parser.h"
#include "tc_common_new/log.h"
#include "tc_common_new/http_client.h"
#include "tc_qt_widget/tc_password_input.h"

namespace tc
{

    InputRemotePwdDialog::InputRemotePwdDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        context_ = ctx;
        setFixedSize(375, 250);
        CreateLayout();
    }

    InputRemotePwdDialog::~InputRemotePwdDialog() = default;

    void InputRemotePwdDialog::CreateLayout() {
        setWindowTitle(tr("Input remote password"));

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

            auto label = new QLabel(this);
            label->setFixedWidth(item_width);
            label->setText("Password");
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            pwd_input_ = new TcPasswordInput(this);
            pwd_input_->setFixedSize(edit_size);

            layout->addWidget(pwd_input_);
            layout->addStretch();

            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(50);

        // sure button
        {
            auto layout = new NoMarginVLayout();
            auto btn_sure = new QPushButton(tr("OK"));
            connect(btn_sure, &QPushButton::clicked, this, [=, this] () {
                this->close();
            });

            layout->addWidget(btn_sure);
            btn_sure->setFixedSize(QSize(item_width, 35));
            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(10);
        root_layout_->addStretch();
    }

    void InputRemotePwdDialog::paintEvent(QPaintEvent *event) {
        TcCustomTitleBarDialog::paintEvent(event);
    }

    QString InputRemotePwdDialog::GetInputPassword() {
        return pwd_input_->GetPassword();
    }

}
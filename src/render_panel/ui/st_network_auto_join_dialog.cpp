//
// Created by RGAA on 2023-08-18.
//

#include "st_network_auto_join_dialog.h"
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
#include "render_panel/spvr_scanner/spvr_scanner.h"
#include "tc_common_new/time_util.h"

namespace tc
{

    StNetworkAutoJoinDialog::StNetworkAutoJoinDialog(const std::shared_ptr<GrApplication>& app, const std::shared_ptr<StNetworkSpvrAccessInfo>& item, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        app_ = app;
        context_ = app_->GetContext();
        item_ = item;
        setFixedSize(375, 475);
        CreateLayout();
    }

    StNetworkAutoJoinDialog::~StNetworkAutoJoinDialog() = default;

    void StNetworkAutoJoinDialog::CreateLayout() {
        setWindowTitle(tcTr("id_found_manager_server"));

        auto item_width = 320;
        auto edit_size = QSize(item_width, 35);

        auto root_layout = new NoMarginHLayout();
        auto content_layout = new NoMarginVLayout();
        root_layout->addStretch();
        root_layout->addLayout(content_layout);
        root_layout->addStretch();
        root_layout_->addLayout(root_layout);

        content_layout->addSpacing(25);

        // 0. spvr info
        {
            auto layout = new NoMarginVLayout();

            auto label = new TcLabel(this);
            label->setFixedWidth(item_width);
            label->SetTextId("id_spvr_server_address");
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto edit = new QLabel(this);
            edit->setText(std::format("{}:{}", item_->spvr_ip_, item_->spvr_port_).c_str());
            edit->setStyleSheet("font-size: 16px; font-weight: 700; color: #2979ff;");
            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();

            content_layout->addLayout(layout);

        }

        content_layout->addSpacing(25);

        // relay info
        {
            auto layout = new NoMarginVLayout();

            auto label = new TcLabel(this);
            label->setFixedWidth(item_width);
            label->SetTextId("id_relay_server_address");
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto edit = new QLabel(this);
            edit->setText(std::format("{}:{}", item_->relay_ip_, item_->relay_port_).c_str());
            edit->setStyleSheet("font-size: 16px; font-weight: 700; color: #2979ff;");
            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();

            content_layout->addLayout(layout);

        }

        content_layout->addSpacing(25);

        // time
        {
            auto layout = new NoMarginVLayout();

            auto label = new TcLabel(this);
            label->setFixedWidth(item_width);
            label->SetTextId("id_timestamp");
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto edit = new QLabel(this);
            edit->setText(std::format("{}", TimeUtil::FormatTimestamp(item_->update_timestamp_)).c_str());
            edit->setStyleSheet("font-size: 16px; font-weight: 700; color: #2979ff;");
            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();

            content_layout->addLayout(layout);

        }

        content_layout->addSpacing(25);

        // sure button
        {
            auto layout = new NoMarginVLayout();
            auto btn_sure = new TcPushButton();
            btn_sure->SetTextId("id_join_in");
            connect(btn_sure, &QPushButton::clicked, this, [=, this] () {
                done(0);
            });

            layout->addWidget(btn_sure);
            btn_sure->setFixedSize(QSize(item_width, 35));

            content_layout->addSpacing(105);
            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(30);
        root_layout_->addStretch();
    }

    void StNetworkAutoJoinDialog::paintEvent(QPaintEvent *event) {
        TcCustomTitleBarDialog::paintEvent(event);
    }

    void StNetworkAutoJoinDialog::closeEvent(QCloseEvent* e) {
        TcCustomTitleBarDialog::closeEvent(e);
        done(1);
    }

}
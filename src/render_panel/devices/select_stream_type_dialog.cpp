//
// Created by RGAA on 19/05/2025.
//

#include "select_stream_type_dialog.h"
#include "no_margin_layout.h"
#include "tc_qt_widget/clickable_widget.h"
#include "tc_label.h"

namespace tc
{

    SelectStreamTypeDialog::SelectStreamTypeDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        setWindowTitle(tcTr("id_create_a_stream"));
        this->setFixedSize(500, 280);

        auto root_layout = new NoMarginHLayout();

        root_layout->addStretch();

        {
            auto widget = new ClickableWidget(this);
            auto layout = new NoMarginVLayout();
            widget->SetRadius(8);
            widget->SetColors(0xf0f0f0, 0xdddddd, 0xcccccc);
            widget->setFixedSize(200, 150);
            widget->setLayout(layout);
            // title
            auto lbl_title = new TcLabel(this);
            lbl_title->setFixedSize(widget->width(), 35);
            lbl_title->SetTextId("id_by_conn_info");
            lbl_title->setStyleSheet("font-size: 14px; font-weight: 700; color: #333333;");
            lbl_title->setAlignment(Qt::AlignCenter);
            layout->addSpacing(10);
            layout->addWidget(lbl_title);

            auto lbl_eg = new QLabel(this);
            lbl_eg->setFixedWidth(widget->width());
            lbl_eg->setAlignment(Qt::AlignCenter);
            lbl_eg->setStyleSheet("font-size: 12px; font-weight: 700; color: #2979ff;");
            lbl_eg->setText("For example:\nlink://6e4e6296\n68e87ab2f4f08d03b...");
            layout->addSpacing(8);
            layout->addWidget(lbl_eg);

            layout->addStretch();

            widget->SetOnClickListener([=, this](auto w) {
                done(1);
            });

            root_layout->addWidget(widget);
        }

        root_layout->addStretch();

        {
            auto layout = new NoMarginVLayout();
            auto widget = new ClickableWidget(this);
            widget->SetRadius(8);
            widget->setFixedSize(200, 150);
            widget->SetColors(0xf0f0f0, 0xdddddd, 0xcccccc);
            widget->setLayout(layout);
            // title
            auto lbl_title = new TcLabel(this);
            lbl_title->setFixedSize(widget->width(), 35);
            lbl_title->SetTextId("id_by_host_port");
            lbl_title->setStyleSheet("font-size: 14px; font-weight: 700; color: #333333;");
            lbl_title->setAlignment(Qt::AlignCenter);
            layout->addSpacing(10);
            layout->addWidget(lbl_title);

            auto lbl_eg = new QLabel(this);
            lbl_eg->setFixedWidth(widget->width());
            lbl_eg->setAlignment(Qt::AlignCenter);
            lbl_eg->setStyleSheet("font-size: 12px; font-weight: 700; color: #2979ff;");
            lbl_eg->setText("For example:\nHost: 192.168.1.5\nPort: 20371       ");
            layout->addSpacing(8);
            layout->addWidget(lbl_eg);

            layout->addStretch();

            widget->SetOnClickListener([=, this](auto w) {
                done(2);
            });

            root_layout->addWidget(widget);
        }

        root_layout->addStretch();

        root_layout_->addLayout(root_layout);
    }

}
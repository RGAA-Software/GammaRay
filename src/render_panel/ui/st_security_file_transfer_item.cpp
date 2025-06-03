//
// Created by RGAA on 30/04/2025.
//

#include "st_security_file_transfer_item.h"
#include "no_margin_layout.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/database/file_transfer_record.h"
#include "tc_common_new/time_util.h"
#include <QLabel>
#include <QPushButton>

namespace tc
{

    const QString kDisplayPluginEnabled = "Enabled";
    const QString kDisplayPluginDisabled = "Disabled";

    StSecurityFileTransferItemWidget::StSecurityFileTransferItemWidget(const std::shared_ptr<GrApplication>& app,
                       const std::shared_ptr<FileTransferRecord>& item_info,
                       QWidget* parent) : QWidget(parent) {
        app_ = app;
        context_ = app->GetContext();
        item_info_ = item_info;

        this->setObjectName("StSecurityFileTransferItemWidget");
        this->setStyleSheet("#StSecurityFileTransferItemWidget {background:#ffffffff;}");

        auto root_layout = new NoMarginVLayout();

        auto content_layout = new NoMarginHLayout();
        root_layout->addStretch();
        root_layout->addLayout(content_layout);
        root_layout->addStretch();

        //content_layout->addSpacing(20);

        auto header_style = R"(font-weight: 500; padding-left: 3px; color: #333333;)";
        auto item_style = R"(font-weight: 500; padding-left: 3px; color: #555555;)";
        auto target_style = item_info_->IsValid() ? item_style : header_style;

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(R"(font-weight: 500; padding-left: 8px; color: #555555;)");
            if (item_info_->IsValid()) {
                lbl->setText(item_info_->conn_type_.c_str());
            }
            else {
                lbl->setText("Connection Type");
            }

            content_layout->addWidget(lbl, 1);
        }

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(target_style);
            if (item_info_->IsValid()) {
                lbl->setText(TimeUtil::FormatTimestamp(item_info_->begin_).c_str());
            }
            else {
                lbl->setText("Start");
            }
            content_layout->addWidget(lbl, 1);
        }

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(target_style);
            if (item_info_->IsValid()) {
                lbl->setText(TimeUtil::FormatTimestamp(item_info_->end_).c_str());
            }
            else {
                lbl->setText("End");
            }
            content_layout->addWidget(lbl, 1);
        }

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(target_style);
            if (item_info_->IsValid()) {
                lbl->setText(item_info_->account_.c_str());
            }
            else {
                lbl->setText("Account");
            }
            content_layout->addWidget(lbl, 1);

        }

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(target_style);
            if (item_info_->IsValid()) {
                lbl->setText(item_info_->controller_device_.c_str());
            }
            else {
                lbl->setText("Controller Device");
            }
            content_layout->addWidget(lbl, 1);
        }

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(target_style);
            if (item_info_->IsValid()) {
                lbl->setText(item_info_->controlled_device_.c_str());
            }
            else {
                lbl->setText("Controlled Device");
            }
            content_layout->addWidget(lbl, 1);
        }

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(target_style);
            if (item_info_->IsValid()) {
                lbl->setText(item_info_->direction_.c_str());
            }
            else {
                lbl->setText("Direction");
            }
            content_layout->addWidget(lbl, 1);
        }

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(target_style);
            if (item_info_->IsValid()) {
                lbl->setText(item_info_->file_detail_.c_str());
            }
            else {
                lbl->setText("File Detail");
            }
            content_layout->addWidget(lbl, 1);
        }


        content_layout->addSpacing(10);

        setLayout(root_layout);
    }

    void StSecurityFileTransferItemWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        if (item_info_->IsValid()) {
            QPen pen;
            pen.setStyle(Qt::PenStyle::DashDotDotLine);
            pen.setColor(0xdddddd);
            painter.setPen(pen);
            int offset = 2;
            painter.drawRoundedRect(QRect(offset, offset, this->width() - offset * 2, this->height() - offset * 2), 5, 5);

            if (enter_) {
                painter.setBrush(QBrush(0xf1f1f1));
            }
            else {
                painter.setBrush(QBrush(0xffffff));
            }
            painter.drawRoundedRect(QRect(offset, offset, this->width() - offset * 2, this->height() - offset * 2), 5, 5);
        }
        else {
            int offset = 2;
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(0xeeeeee));
            painter.drawRoundedRect(QRect(offset, offset, this->width() - offset * 2, this->height() - offset * 2), 5, 5);
        }
        QWidget::paintEvent(event);
    }

    void StSecurityFileTransferItemWidget::UpdateStatus() {
        if (!lbl_enabled_) {
            return;
        }



    }

    void StSecurityFileTransferItemWidget::UpdatePluginStatus(bool enabled) {
        if (enabled) {
            lbl_enabled_->setText(kDisplayPluginEnabled);
            lbl_enabled_->setStyleSheet(R"(font-weight: 700; color: #555555;)");
        }
        else {
            lbl_enabled_->setText(kDisplayPluginDisabled);
            lbl_enabled_->setStyleSheet(R"(font-weight: 700; color: #ff2200;)");
        }
    }

    void StSecurityFileTransferItemWidget::SwitchPluginStatusInner(bool enabled) {

    }

    void StSecurityFileTransferItemWidget::enterEvent(QEnterEvent *event) {
        enter_ = true;
        repaint();
    }

    void StSecurityFileTransferItemWidget::leaveEvent(QEvent *event) {
        enter_ = false;
        repaint();
    }

}
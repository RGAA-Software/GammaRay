//
// Created by RGAA on 30/04/2025.
//

#include "st_plugin_item_widget.h"
#include "no_margin_layout.h"
#include "st_plugins.h"
#include "tc_message.pb.h"
#include <QLabel>
#include <QPushButton>

namespace tc
{

    StPluginItemWidget::StPluginItemWidget(const std::shared_ptr<GrApplication>& app,
                       const std::shared_ptr<PluginItemInfo>& item_info,
                       QWidget* parent) : QWidget(parent) {

        item_info_ = item_info;

        this->setObjectName("StPluginItemWidget");
        this->setStyleSheet("#StPluginItemWidget {background:#ffffffff;}");

        auto root_layout = new NoMarginVLayout();

        auto content_layout = new NoMarginHLayout();
        root_layout->addStretch();
        root_layout->addLayout(content_layout);
        root_layout->addStretch();

        // icon
        {
            auto icon = new QLabel(this);
            icon->setFixedSize(30, 30);
            QString style = R"(background-image: url(:resources/image/ic_plugin.svg);
                        background-repeat: no-repeat;
                        background-position: center;
                    )";
            icon->setStyleSheet(style);
            content_layout->addSpacing(10);
            content_layout->addWidget(icon);
            content_layout->addSpacing(20);
        }

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(R"(font-weight: 700;)");
            lbl->setFixedWidth(180);
            lbl->setText(item_info->info_->name().c_str());
            content_layout->addWidget(lbl);
            content_layout->addSpacing(20);
        }

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(R"(font-size: 12px;)");
            lbl->setFixedWidth(300);
            lbl->setText(item_info->info_->desc().c_str());
            content_layout->addWidget(lbl);
            content_layout->addSpacing(20);
        }

        {
            auto lbl = new QLabel(this);
            lbl_enabled_ = lbl;
            lbl->setFixedWidth(120);
            content_layout->addWidget(lbl);
            content_layout->addSpacing(20);
            SetEnabled(item_info->info_->enabled());
        }

        content_layout->addStretch();

        auto size = QSize(75, 30);
        {
            auto btn = new QPushButton(this);
            btn->setProperty("class", "danger");
            btn->setFixedSize(size);
            btn->setText("Disable");
            content_layout->addWidget(btn);
            content_layout->addSpacing(10);
        }
        {
            auto btn = new QPushButton(this);
            btn->setFixedSize(size);
            btn->setText("Enable");
            content_layout->addWidget(btn);
            content_layout->addSpacing(10);
        }

        content_layout->addSpacing(10);

        setLayout(root_layout);
    }

    void StPluginItemWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        QPen pen;
        pen.setStyle(Qt::PenStyle::DashDotDotLine);
        pen.setColor(0xdddddd);
        painter.setPen(pen);
        int offset = 3;
        painter.drawRoundedRect(QRect(offset, offset, this->width()-offset*2, this->height()-offset*2), 5, 5);

        QWidget::paintEvent(event);
    }

    void StPluginItemWidget::UpdateStatus(const std::shared_ptr<PluginItemInfo>& item_info) {
        if (!lbl_enabled_) {
            return;
        }
        if (item_info->info_->enabled() == item_info_->info_->enabled()) {
            return;
        }

        item_info_ = item_info;
        SetEnabled(item_info_->info_->enabled());

    }

    void StPluginItemWidget::SetEnabled(bool enabled) {
        if (enabled) {
            lbl_enabled_->setText("Enabled");
            lbl_enabled_->setStyleSheet(R"(font-weight: 700; color: #555555;)");
        }
        else {
            lbl_enabled_->setText("Disabled");
            lbl_enabled_->setStyleSheet(R"(font-weight: 700; color: #ff2200;)");
        }
    }

}
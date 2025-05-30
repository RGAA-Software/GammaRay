//
// Created by RGAA on 30/04/2025.
//

#include "tab_profile_device_item.h"
#include "no_margin_layout.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_app_messages.h"
#include "tc_account_sdk/acc_device.h"
#include <QLabel>
#include <QPushButton>

namespace tc
{

    const QString kDisplayPluginEnabled = "Enabled";
    const QString kDisplayPluginDisabled = "Disabled";

    TabProfileDeviceItemWidget::TabProfileDeviceItemWidget(const std::shared_ptr<GrApplication>& app,
                       const std::shared_ptr<AccountDevice>& item_info,
                       QWidget* parent) : QWidget(parent) {
        app_ = app;
        context_ = app->GetContext();
        item_info_ = item_info;

        this->setObjectName("TabProfileDeviceItemWidget");
        this->setStyleSheet("#TabProfileDeviceItemWidget {background:#ffffffff;}");

        auto root_layout = new NoMarginVLayout();

        auto content_layout = new NoMarginHLayout();
        root_layout->addLayout(content_layout);

        //content_layout->addSpacing(20);

        auto header_style = R"(font-weight: 500; padding-left: 3px; color: #333333;)";
        auto item_style = R"(font-weight: 500; padding-left: 3px; color: #555555;)";
        {
            auto lbl = new QLabel(this);
            int icon_size = 30;
            lbl->setFixedSize(icon_size, icon_size);
            static auto icon = QPixmap::fromImage(QImage(":/resources/image/ic_windows_direct.svg"));
            if (icon.width() != icon_size) {
                icon = icon.scaled(icon_size, icon_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            lbl->setPixmap(icon);
            content_layout->addSpacing(15);
            content_layout->addWidget(lbl, 0, Qt::AlignVCenter);
        }

        {
            auto layout = new NoMarginVLayout();
            {
                auto title = new QLabel(this);
                title->setText(item_info->device_custom_name_.c_str());
                title->setStyleSheet(R"(font-weight: 700; font-size: 15px; color: #333333;)");
                layout->addSpacing(10);
                layout->addWidget(title);

            }

            layout->addSpacing(3);

            {
                auto sub_layout = new NoMarginHLayout();
                // os
                {
                    auto lbl = new QLabel(this);
                    lbl->setText(std::format("{} {}", item_info->device_os_, item_info->device_desktop_name_).c_str());
                    lbl->setStyleSheet(R"(font-size: 13px; color: #555555;)");
                    sub_layout->addWidget(lbl);
                }
                {
                    sub_layout->addStretch();
                }
                layout->addLayout(sub_layout);
            }

            layout->addStretch();
            content_layout->addSpacing(15);
            content_layout->addLayout(layout);
        }

        content_layout->addStretch();

        setLayout(root_layout);
    }

    void TabProfileDeviceItemWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        if (item_info_->IsValid()) {
            QPen pen;
            pen.setStyle(Qt::PenStyle::DotLine);
            pen.setColor(0xcccccc);
            painter.setPen(pen);
            int offset = 2;
            painter.drawRoundedRect(QRect(offset, offset, this->width() - offset * 2, this->height() - offset * 2), 5, 5);

            if (pressed_) {
                painter.setBrush(QBrush(0xe1e1e1));
            }
            else if (enter_) {
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

    void TabProfileDeviceItemWidget::enterEvent(QEnterEvent *event) {
        enter_ = true;
        repaint();
    }

    void TabProfileDeviceItemWidget::leaveEvent(QEvent *event) {
        enter_ = false;
        repaint();
    }

    void TabProfileDeviceItemWidget::mousePressEvent(QMouseEvent *event) {
        pressed_ = true;
        repaint();
    }

    void TabProfileDeviceItemWidget::mouseReleaseEvent(QMouseEvent *event) {
        pressed_ = false;
        repaint();
        if (click_listener_) {
            click_listener_(this, item_info_);
        }
    }

    void TabProfileDeviceItemWidget::SetOnItemClickListener(OnItemValueClickListener<AccountDevice>&& listener) {
        click_listener_ = listener;
    }

}
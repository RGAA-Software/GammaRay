//
// Created by RGAA on 21/09/2025.
//

#include "hw_info_widget.h"
#include "no_margin_layout.h"
#include "tc_label.h"
#include "hw_info.h"

namespace tc
{

    static QString GetItemIconStyleSheet(const QString &url) {
        QString style = R"(background-image: url(%1);
                        background-repeat: no-repeat;
                        background-position: center;
                    )";
        return style.arg(url);
    }

    HWInfoWidget::HWInfoWidget(QWidget* parent) : QWidget(parent) {

        auto content_root = new NoMarginHLayout();

        // LEFT
        auto label_width = 140;
        auto value_width = 185;
        int margin_left = 20;
        auto icon_size = 35;
        {
            auto layout = new NoMarginVLayout();
            // title margin
            layout->addSpacing(3);

            content_root->addLayout(layout);
            // Server Status
            {
                auto item_layout = new NoMarginHLayout();
                auto title = new TcLabel(this);
                title->SetTextId("id_tab_hardware");
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 22px; font-weight:700;)");
                item_layout->addSpacing(margin_left + 9);
                item_layout->addWidget(title);
                item_layout->addStretch();
                layout->addLayout(item_layout);
                layout->addSpacing(8);
            }

            // PC Name
            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(icon_size, icon_size);
                icon->setStyleSheet(GetItemIconStyleSheet(":/resources/image/ic_computer.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_hw_pc_name");
                label->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_pc_name_ = value;
                value->setFixedSize(value_width, 40);
                value->setText("--");
                value->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(value);
                item_layout->addStretch();
                layout->addLayout(item_layout);
            }

            // OS Version
            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(icon_size, icon_size);
                icon->setStyleSheet(GetItemIconStyleSheet(":/resources/image/ic_os_version.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_hw_os_version");
                label->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_os_version_ = value;
                value->setFixedSize(value_width, 40);
                value->setText("--");
                value->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(value);
                item_layout->addStretch();
                layout->addLayout(item_layout);
            }
            layout->addStretch();
        }

        content_root->addStretch();

        setLayout(content_root);
    }

    void HWInfoWidget::OnSysInfoCallback(const std::shared_ptr<SysInfo>& si) {
        sys_info_hist_.push_back(si);
        if (sys_info_hist_.size() > 180) {
            sys_info_hist_.pop_front();
        }

        QMetaObject::invokeMethod(this, [=, this]() {
            this->RefreshInternal();
        });
    }

    void HWInfoWidget::paintEvent(QPaintEvent *event) {
//        QPainter painter(this);
//        painter.setBrush(QBrush(0xbbbbbb));
//        painter.drawRect(this->rect());
    }

    void HWInfoWidget::RefreshInternal() {
        if (sys_info_hist_.empty()) {
            return;
        }
        auto sys_info = sys_info_hist_.back();
        lbl_pc_name_->setText(sys_info->os_.sys_host_name_.c_str());
        lbl_os_version_->setText(std::format("{}", sys_info->os_.sys_kernel_).c_str());
    }

}
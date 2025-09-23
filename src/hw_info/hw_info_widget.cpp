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
        auto label_width = 130;
        auto value_width = 230;
        int margin_left = 20;
        auto icon_size = 30;
        int lbl_margin_left = 5;
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
                item_layout->addSpacing(margin_left + 7);
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
                label->setStyleSheet("font-size: 13px;");
                item_layout->addSpacing(lbl_margin_left);
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_pc_name_ = value;
                value->setFixedSize(value_width, 40);
                value->setText("--");
                value->setStyleSheet("font-size: 13px;");
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
                label->setStyleSheet("font-size: 13px;");
                item_layout->addSpacing(lbl_margin_left);
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_os_version_ = value;
                value->setFixedSize(value_width, 40);
                value->setText("--");
                value->setStyleSheet("font-size: 13px;");
                item_layout->addWidget(value);
                item_layout->addStretch();
                layout->addLayout(item_layout);
            }

            // CPU Brand
            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(icon_size, icon_size);
                icon->setStyleSheet(GetItemIconStyleSheet(":/resources/image/ic_hw_cpu.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_hw_cpu_brand");
                label->setStyleSheet("font-size: 13px;");
                item_layout->addSpacing(lbl_margin_left);
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_cpu_ = value;
                value->setFixedSize(value_width, 40);
                value->setText("--");
                value->setStyleSheet("font-size: 13px;");
                item_layout->addWidget(value);
                item_layout->addStretch();
                layout->addLayout(item_layout);
            }

            // Memory
            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(icon_size, icon_size);
                icon->setStyleSheet(GetItemIconStyleSheet(":/resources/image/ic_memory.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_hw_memory");
                label->setStyleSheet("font-size: 13px;");
                item_layout->addSpacing(lbl_margin_left);
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_memory_ = value;
                value->setFixedSize(value_width, 40);
                value->setText("--");
                value->setStyleSheet("font-size: 13px;");
                item_layout->addWidget(value);
                item_layout->addStretch();
                layout->addLayout(item_layout);
            }

            // Running
            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(icon_size, icon_size);
                icon->setStyleSheet(GetItemIconStyleSheet(":/resources/image/ic_running.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_hw_running");
                label->setStyleSheet("font-size: 13px;");
                item_layout->addSpacing(lbl_margin_left);
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_uptime_ = value;
                value->setFixedSize(value_width, 40);
                value->setText("--");
                value->setStyleSheet("font-size: 13px;");
                item_layout->addWidget(value);
                item_layout->addStretch();
                layout->addLayout(item_layout);
            }

            // Disk
            disk_widget_ = new QWidget(this);
            layout->addWidget(disk_widget_);

            // Disk
            net_widget_ = new QWidget(this);
            layout->addWidget(net_widget_);

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
        lbl_os_version_->setText(std::format("{}", sys_info->os_.sys_os_long_version_).c_str());

        QString cpu_brand = QString::fromStdString(sys_info->cpu_.brand_);
        cpu_brand = cpu_brand.replace("(R)", "").replace("(TM)", "").replace("@ ", "");
        lbl_cpu_->setText(cpu_brand);

        {
            auto percent = sys_info->mem_.used_gb_ * 100 / sys_info->mem_.total_gb_;
            lbl_memory_->setText(std::format("Used: {}GB, Total: {}GB, {}%", sys_info->mem_.used_gb_, sys_info->mem_.total_gb_, percent).c_str());
        }
        lbl_uptime_->setText(sys_info->uptime_.c_str());

        //
        if (disk_widget_->children().empty() && !sys_info->disks_.empty()) {
           GenDiskList(sys_info);
        }
        {
            int index = 0;
            if (sys_info->disks_.size() <= lbl_disks_.size()) {
                for (const auto &disk: sys_info->disks_) {
                    auto mount = QString::fromStdString(disk.mount_on_);
                    mount = mount.replace("\\", "").replace(":", "");
                    auto used_gb = disk.total_gb_ - disk.available_gb_;
                    auto percent = (used_gb * 100) / disk.total_gb_;
                    lbl_disks_[index].title_->setText(std::format("{}({}[{}])", tcTr("id_hw_disk").toStdString(), mount.toStdString(), disk.disk_type_).c_str());
                    lbl_disks_[index].value_->setText(std::format("Used: {}GB, Total: {}GB, {}%", used_gb, disk.total_gb_, percent).c_str());
                    index++;
                }
            }
        }

        //
        if (net_widget_->children().empty() && !sys_info->networks_.empty()) {
            GenNetworkList(sys_info);
        }
        {
            int index = 0;
            if (lbl_networks_.size() <= sys_info->networks_.size()) {
                for (const auto &network: sys_info->networks_) {
                    QString name = QString::fromStdString(network.name_);
                    if (name.contains("WSL") || name.contains("vEthernet")) {
                        continue;
                    }

                    lbl_networks_[index].title_->setText(network.name_.c_str());
                    std::string ip;
                    if (!network.ip_networks_.empty()) {
                        ip = network.ip_networks_[0].addr_;
                    }
                    lbl_networks_[index].value_->setText(std::format("{}, {}", ip, network.mac_).c_str());
                    index++;
                }
            }
        }
    }

    void HWInfoWidget::GenDiskList(const std::shared_ptr<SysInfo>& sys_info) {
        auto root_layout = new NoMarginVLayout();
        for (const auto& disk : sys_info->disks_) {
            auto label_width = 130;
            auto value_width = 230;
            int margin_left = 20;
            auto icon_size = 32;
            int lbl_margin_left = 5;
            auto item_layout = new NoMarginHLayout();
            item_layout->addSpacing(margin_left);
            auto icon = new TcLabel(this);
            icon->setFixedSize(icon_size, icon_size);
            icon->setStyleSheet(GetItemIconStyleSheet(":/resources/image/ic_hard_disk.svg"));
            item_layout->addWidget(icon);

            auto label = new TcLabel(this);
            label->setFixedSize(label_width, 40);
            label->SetTextId("id_hw_disk");
            label->setStyleSheet("font-size: 13px;");
            item_layout->addSpacing(lbl_margin_left);
            item_layout->addWidget(label);

            auto value = new TcLabel(this);
            value->setFixedSize(value_width, 40);
            value->setText("--");
            value->setStyleSheet("font-size: 13px;");
            item_layout->addWidget(value);
            item_layout->addStretch();
            root_layout->addLayout(item_layout);

            lbl_disks_.push_back(EditableLine {
                .title_ = label,
                .value_ = value,
            });
        }
        disk_widget_->setLayout(root_layout);
    }

    void HWInfoWidget::GenNetworkList(const std::shared_ptr<SysInfo> &sys_info) {
        //:/icons/ic_network.svg
        auto root_layout = new NoMarginVLayout();
        for (const auto& network : sys_info->networks_) {
            QString name = QString::fromStdString(network.name_);
            if (name.contains("WSL") || name.contains("vEthernet")) {
                continue;
            }
            auto label_width = 130;
            auto value_width = 230;
            int margin_left = 20;
            auto icon_size = 32;
            int lbl_margin_left = 5;
            auto item_layout = new NoMarginHLayout();
            item_layout->addSpacing(margin_left);
            auto icon = new TcLabel(this);
            icon->setFixedSize(icon_size, icon_size);
            icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_network.svg"));
            item_layout->addWidget(icon);

            auto label = new TcLabel(this);
            label->setFixedSize(label_width, 40);
            label->SetTextId("id_hw_ethernet");
            label->setStyleSheet("font-size: 13px;");
            item_layout->addSpacing(lbl_margin_left);
            item_layout->addWidget(label);

            auto value = new TcLabel(this);
            value->setFixedSize(value_width, 40);
            value->setText("--");
            value->setStyleSheet("font-size: 13px;");
            item_layout->addWidget(value);
            item_layout->addStretch();
            root_layout->addLayout(item_layout);

            lbl_networks_.push_back(EditableLine {
                .title_ = label,
                .value_ = value,
            });
        }
        net_widget_->setLayout(root_layout);
    }

}
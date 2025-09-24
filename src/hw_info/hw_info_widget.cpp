//
// Created by RGAA on 21/09/2025.
//

#include "hw_info_widget.h"
#include "no_margin_layout.h"
#include "tc_label.h"
#include "hw_info.h"
#include "hw_stat_chart.h"
#include "hw_cpu_detail_widget.h"
#include "tc_common_new/num_formatter.h"

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
        auto value_width = 270;
        int margin_left = 20;
        auto icon_size = 30;
        int lbl_margin_left = 5;
        int item_height = 32;
        {
            auto layout = new NoMarginVLayout();

            content_root->addLayout(layout);
            // Server Status
            {
                auto item_layout = new NoMarginHLayout();
                auto title = new TcLabel(this);
                title->SetTextId("id_tab_hardware");
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 22px; font-weight:700;)");
                item_layout->addSpacing(margin_left + 3);
                item_layout->addWidget(title);
                item_layout->addStretch();
                layout->addLayout(item_layout);
                layout->addSpacing(6);
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
                label->setFixedSize(label_width, item_height);
                label->SetTextId("id_hw_pc_name");
                label->setStyleSheet("font-size: 13px;");
                item_layout->addSpacing(lbl_margin_left);
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_pc_name_ = value;
                value->setFixedSize(value_width, item_height);
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
                label->setFixedSize(label_width, item_height);
                label->SetTextId("id_hw_os_version");
                label->setStyleSheet("font-size: 13px;");
                item_layout->addSpacing(lbl_margin_left);
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_os_version_ = value;
                value->setFixedSize(value_width, item_height);
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
                label->setFixedSize(label_width, item_height);
                label->SetTextId("id_hw_cpu_brand");
                label->setStyleSheet("font-size: 13px;");
                item_layout->addSpacing(lbl_margin_left);
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_cpu_ = value;
                value->setFixedSize(value_width, item_height);
                value->setText("--");
                value->setStyleSheet("font-size: 13px;");
                item_layout->addWidget(value);
                item_layout->addStretch();
                layout->addLayout(item_layout);
            }

            // CPU Info
            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(icon_size, icon_size);
                icon->setStyleSheet(GetItemIconStyleSheet(":/resources/image/ic_hw_cpu.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, item_height);
                label->SetTextId("id_hw_cpu_info");
                label->setStyleSheet("font-size: 13px;");
                item_layout->addSpacing(lbl_margin_left);
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_cpu_info_ = value;
                value->setFixedSize(value_width, item_height);
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
                label->setFixedSize(label_width, item_height);
                label->SetTextId("id_hw_memory");
                label->setStyleSheet("font-size: 13px;");
                item_layout->addSpacing(lbl_margin_left);
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_memory_ = value;
                value->setFixedSize(value_width, item_height);
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
                icon->setFixedSize(icon_size, item_height);
                icon->setStyleSheet(GetItemIconStyleSheet(":/resources/image/ic_running.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, item_height);
                label->SetTextId("id_hw_running");
                label->setStyleSheet("font-size: 13px;");
                item_layout->addSpacing(lbl_margin_left);
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                lbl_uptime_ = value;
                value->setFixedSize(value_width, icon_size);
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

            layout->addSpacing(10);

            //
            auto chart_width = 220;
            auto chart_height = 110;
            {
                auto chart_layout = new NoMarginHLayout();
                chart_layout->addSpacing(margin_left);
                {
                    auto chart = new HWStatChart(this);
                    chart_cpu_usage_ = chart;
                    chart->SetTitle("CPU Usage");
                    chart->SetYAxisDesc("100%");
                    chart->setFixedSize(chart_width, chart_height);
                    chart_layout->addWidget(chart);
                }

                chart_layout->addSpacing(10);

                {
                    auto chart = new HWStatChart(this);
                    chart_cpu_freq_ = chart;
                    chart->SetTitle("CPU Speed");
                    chart->SetYAxisDesc("6GHz");
                    chart->setFixedSize(chart_width, chart_height);
                    chart_layout->addWidget(chart);
                }

                chart_layout->addStretch();
                layout->addLayout(chart_layout);
            }

            layout->addSpacing(10);

            {
                auto chart_layout = new NoMarginHLayout();
                chart_layout->addSpacing(margin_left);
                {
                    auto chart = new HWStatChart(this);
                    chart_memory_ = chart;
                    chart->SetTitle("Memory");
                    chart->setFixedSize(chart_width, chart_height);
                    chart_layout->addWidget(chart);
                }

                chart_layout->addSpacing(10);

                {
                    auto chart = new HWStatChart(this);
                    chart_net_received_speed_ = chart;
                    chart->SetTitle("Recv");
                    chart->SetChartType(HWStatChartType::kLine);
                    chart->SetYAxisDesc("50MB/s");
                    chart->setFixedSize(chart_width, chart_height);
                    chart_layout->addWidget(chart);
                }

                chart_layout->addStretch();
                layout->addLayout(chart_layout);
            }
            layout->addSpacing(10);
            {
                auto chart_layout = new NoMarginHLayout();
                chart_layout->addSpacing(margin_left);
                {
                    auto chart = new HWStatChart(this);
                    chart_net_send_speed_ = chart;
                    chart->SetChartType(HWStatChartType::kLine);
                    chart->SetTitle("Send");
                    chart->SetYAxisDesc("50MB/s");
                    chart->setFixedSize(chart_width, chart_height);
                    chart_layout->addWidget(chart);
                }

                chart_layout->addSpacing(10);

//                {
//                    auto chart = new HWStatChart(this);
//                    chart_net_speed_ = chart;
//                    chart->SetChartType(HWStatChartType::kLine);
//                    chart->SetYAxisDesc("100MB/s");
//                    chart->setFixedSize(chart_width, chart_height);
//                    chart_layout->addWidget(chart);
//                }

                chart_layout->addStretch();
                layout->addLayout(chart_layout);
            }

            layout->addStretch();
        }

        // right
        {
            auto layout = new NoMarginVLayout();
            content_root->addLayout(layout);

            // Detail
            {
                auto item_layout = new NoMarginHLayout();
                auto title = new TcLabel(this);
                title->setFixedWidth(700);
                title->SetTextId("id_hw_detailed_info");
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 22px; font-weight:700;)");
                item_layout->addWidget(title);
                item_layout->addStretch();
                layout->addLayout(item_layout);
                layout->addSpacing(6);
            }

            // CPU List
            {
                auto cpu_list = new HWCpuDetailWidget(this);
                cpu_list->setFixedHeight(250);
                detail_widget_ = cpu_list;
                layout->addWidget(cpu_list);
            }

            layout->addStretch();
        }

        content_root->addSpacing(20);
        setLayout(content_root);
    }

    void HWInfoWidget::OnSysInfoCallback(const std::shared_ptr<SysInfo>& si) {
        sys_info_hist_.push_back(si);
        if (sys_info_hist_.size() > 60) {
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

        auto cpu_info = std::format("{} Cores, Base: {}GHz, Now: {}GHz",
                                    sys_info->cpu_.cpus_.size(), NumFormatter::Round2DecimalPlaces(sys_info->cpu_.base_frequency_),
                                    NumFormatter::Round2DecimalPlaces(sys_info->cpu_.current_frequency_));
        lbl_cpu_info_->setText(cpu_info.c_str());

        //
        detail_widget_->UpdateCpusInfo(sys_info->cpu_.cpus_);

        //
        chart_memory_->SetYAxisDesc(std::format("{}GB", sys_info->mem_.total_gb_).c_str());

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

        {
            std::vector<float> cpu_usages;
            std::vector<float> cpu_freq;
            std::vector<float> memory_usages;
            for (const auto& info : sys_info_hist_) {
                cpu_usages.push_back(info->cpu_.usage_ / 100.0f);
                cpu_freq.push_back(info->cpu_.current_frequency_ * 1.0f / 6);
                memory_usages.push_back(info->mem_.used_gb_ * 1.0f / info->mem_.total_gb_);
            }


            chart_cpu_usage_->SetTitle(std::format("CPU Usage({}%)", NumFormatter::Round2DecimalPlaces(sys_info->cpu_.usage_)).c_str());
            chart_cpu_usage_->UpdateValues(cpu_usages);

            chart_cpu_freq_->SetTitle(std::format("CPU Speed({}GHz)", NumFormatter::Round2DecimalPlaces(sys_info->cpu_.current_frequency_)).c_str());
            chart_cpu_freq_->UpdateValues(cpu_freq);

            chart_memory_->SetTitle(std::format("Memory({}GB)", sys_info->mem_.used_gb_).c_str());
            chart_memory_->UpdateValues(memory_usages);
        }

        if (sys_info_hist_.size() >= 2) {
            std::vector<float> received_speed;
            std::vector<float> send_speed;
            uint32_t latest_received_speed = 0;
            uint32_t latest_send_speed = 0;
            for (int i = 0; i < sys_info_hist_.size(); i++) {
                auto next_idx = i + 1;
                if (next_idx >= sys_info_hist_.size()) {
                    break;
                }
                const auto& nts = sys_info_hist_[i]->networks_;
                if (nts.empty()) {
                    break;
                }
                const auto& nt_info = nts[0];
                auto recv = nt_info.received_data_;
                auto sent = nt_info.sent_data_;

                const auto& next_nts = sys_info_hist_[next_idx]->networks_;
                if (next_nts.empty()) {
                    break;
                }
                const auto& next_nt_info = next_nts[0];
                auto next_recv = next_nt_info.received_data_;
                auto next_sent = next_nt_info.sent_data_;

                // 50MB/s Max
                {
                    latest_received_speed = next_recv - recv;
                    auto speed = latest_received_speed * 1.0f / 1024 / 1024 / 50;
                    received_speed.push_back(speed);
                }
                {
                    latest_send_speed = next_sent - sent;
                    auto speed = latest_send_speed * 1.0f / 1024 / 1024 / 50;
                    send_speed.push_back(speed);
                }
            }

            if (!received_speed.empty()) {
                chart_net_received_speed_->SetTitle(
                        std::format("Recv({})", NumFormatter::FormatSpeed(latest_received_speed)).c_str());
            }
            chart_net_received_speed_->UpdateValues(received_speed);
            if (!send_speed.empty()) {
                chart_net_send_speed_->SetTitle(
                        std::format("Send({})", NumFormatter::FormatSpeed(latest_send_speed)).c_str());
            }
            chart_net_send_speed_->UpdateValues(send_speed);
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
            int item_height = 32;
            auto item_layout = new NoMarginHLayout();
            item_layout->addSpacing(margin_left);
            auto icon = new TcLabel(this);
            icon->setFixedSize(icon_size, icon_size);
            icon->setStyleSheet(GetItemIconStyleSheet(":/resources/image/ic_hard_disk.svg"));
            item_layout->addWidget(icon);

            auto label = new TcLabel(this);
            label->setFixedSize(label_width, item_height);
            label->SetTextId("id_hw_disk");
            label->setStyleSheet("font-size: 13px;");
            item_layout->addSpacing(lbl_margin_left);
            item_layout->addWidget(label);

            auto value = new TcLabel(this);
            value->setFixedSize(value_width, item_height);
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
            int item_height = 32;
            auto item_layout = new NoMarginHLayout();
            item_layout->addSpacing(margin_left);
            auto icon = new TcLabel(this);
            icon->setFixedSize(icon_size, icon_size);
            icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_network.svg"));
            item_layout->addWidget(icon);

            auto label = new TcLabel(this);
            label->setFixedSize(label_width, item_height);
            label->SetTextId("id_hw_ethernet");
            label->setStyleSheet("font-size: 13px;");
            item_layout->addSpacing(lbl_margin_left);
            item_layout->addWidget(label);

            auto value = new TcLabel(this);
            value->setFixedSize(value_width, item_height);
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
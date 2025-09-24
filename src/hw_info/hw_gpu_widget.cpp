//
// Created by RGAA on 24/09/2025.
//

#include "hw_gpu_widget.h"
#include <QPainter>
#include "no_margin_layout.h"
#include "hw_stat_chart.h"

namespace tc
{

    HWGpuWidget::HWGpuWidget(QWidget* parent) {
        auto root_layout = new NoMarginVLayout();
        root_layout->addStretch();

        auto content_layout = new NoMarginHLayout();
        root_layout->addLayout(content_layout);

        auto chart_size = QSize(164, 90);
        auto gap = 5;

        content_layout->addSpacing(gap);

        {
            auto chart = new HWStatChart(this);
            usage_chart_ = chart;
            chart->SetYAxisDesc("100%");
            chart->setFixedSize(chart_size);
            chart->SetTitle("Usage");
            content_layout->addWidget(chart);
        }

        content_layout->addSpacing(gap);

        {
            auto chart = new HWStatChart(this);
            encoder_chart_ = chart;
            chart->SetYAxisDesc("100%");
            chart->setFixedSize(chart_size);
            chart->SetTitle("Encoder");
            content_layout->addWidget(chart);
        }

        content_layout->addSpacing(gap);

        {
            auto chart = new HWStatChart(this);
            memory_chart_ = chart;
            chart->SetYAxisDesc("100%");
            chart->SetTitle("Memory");
            chart->setFixedSize(chart_size);
            content_layout->addWidget(chart);
        }

        content_layout->addSpacing(gap);

        {
            auto chart = new HWStatChart(this);
            temp_chart_ = chart;
            chart->SetYAxisDesc("100℃");
            chart->SetTitle("Temperature");
            chart->setFixedSize(chart_size);
            content_layout->addWidget(chart);
        }

        content_layout->addStretch();

        root_layout->addStretch();
        setLayout(root_layout);
    }

    void HWGpuWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(0xfafafa));
        painter.drawRoundedRect(this->rect(), 10, 10);
    }

    void HWGpuWidget::UpdateGpuInfo(const SysGpuInfo& info) {
        info_ = info;
        constexpr int max_values = 60;
        usages_.push_back(info.gpu_utilization_ / 100.0f);
        if (usages_.size() > max_values) {
            usages_.erase(usages_.begin());
        }
        usage_chart_->UpdateValues(usages_);

        encoders_.push_back(info.encoder_utilization_ / 100.0f);
        if (encoders_.size() > max_values) {
            encoders_.erase(encoders_.begin());
        }
        encoder_chart_->UpdateValues(encoders_);

        memories_.push_back(info.mem_utilization_ / 100.0f);
        if (memories_.size() > max_values) {
            memories_.erase(memories_.begin());
        }
        memory_chart_->UpdateValues(memories_);

        temperatures_.push_back(info.temperature_ / 100.0f);
        if (temperatures_.size() > max_values) {
            temperatures_.erase(temperatures_.begin());
        }
        temp_chart_->UpdateValues(temperatures_);
    }

}
//
// Created by RGAA on 24/09/2025.
//

#include "hw_cpu_detail_item.h"
#include "no_margin_layout.h"

namespace tc
{

    HWCpuDetailItem::HWCpuDetailItem(const QString& title, QWidget* parent) : QWidget(parent) {
        auto layout = new NoMarginVLayout();
        chart_ = new HWStatChart(this);
        chart_->SetTitle(title);
        layout->addWidget(chart_);
        setLayout(layout);
    }

    void HWCpuDetailItem::UpdateValue(float value) {
        values_.push_back(value);
        if (values_.size() >= 60) {
            values_.erase(values_.begin());
        }
        chart_->UpdateValues(values_);
    }

}
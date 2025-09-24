//
// Created by RGAA on 24/09/2025.
//

#ifndef GAMMARAYPREMIUM_HW_CPU_DETAIL_ITEM_H
#define GAMMARAYPREMIUM_HW_CPU_DETAIL_ITEM_H

#include <vector>
#include <QWidget>
#include "hw_info.h"
#include "hw_stat_chart.h"

namespace tc
{
    class HWCpuDetailItem : public QWidget {
    public:
        HWCpuDetailItem(const QString& title, QWidget* parent = nullptr);
        void UpdateValue(float value);

    private:
        HWStatChart* chart_ = nullptr;
        std::vector<float> values_;
    };
}

#endif //GAMMARAYPREMIUM_HW_CPU_DETAIL_ITEM_H

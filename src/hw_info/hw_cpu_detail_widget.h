//
// Created by RGAA on 24/09/2025.
//

#ifndef GAMMARAYPREMIUM_HW_CPU_DETAIL_WIDGET_H
#define GAMMARAYPREMIUM_HW_CPU_DETAIL_WIDGET_H

#include <vector>
#include <QWidget>
#include <QListWidget>
#include "hw_info.h"
#include "hw_cpu_detail_item.h"

namespace tc
{

    using SingleCpuInfoHist = std::vector<SysSingleCpuInfo>;

    class HWCpuDetailWidget : public QWidget {
    public:
        HWCpuDetailWidget(QWidget* parent = nullptr);
        void UpdateCpusInfo(const std::vector<SysSingleCpuInfo>& cpus);
        QListWidgetItem* AddItem(const SysSingleCpuInfo& info, int index);
    private:
        std::vector<SingleCpuInfoHist> cpus_hist_;
        QListWidget* apps_list_ = nullptr;
        bool init_ = false;
    };

}

#endif //GAMMARAYPREMIUM_HW_CPU_DETAIL_WIDGET_H

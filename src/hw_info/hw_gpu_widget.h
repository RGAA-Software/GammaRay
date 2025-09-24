//
// Created by RGAA on 24/09/2025.
//

#ifndef GAMMARAYPREMIUM_HW_GPU_WIDGET_H
#define GAMMARAYPREMIUM_HW_GPU_WIDGET_H

#include <vector>
#include <QWidget>
#include "hw_info.h"

namespace tc
{

    class HWStatChart;

    class HWGpuWidget : public QWidget {
    public:
        HWGpuWidget(QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;
        void UpdateGpuInfo(const SysGpuInfo& info);

    private:
        SysGpuInfo info_;
        std::vector<float> usages_;
        HWStatChart* usage_chart_ = nullptr;

        std::vector<float> encoders_;
        HWStatChart* encoder_chart_ = nullptr;

        std::vector<float> memories_;
        HWStatChart* memory_chart_ = nullptr;

        std::vector<float> temperatures_;
        HWStatChart* temp_chart_ = nullptr;

    };

}

#endif //GAMMARAYPREMIUM_HW_GPU_WIDGET_H

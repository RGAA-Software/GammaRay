//
// Created by RGAA on 23/09/2025.
//

#ifndef GAMMARAYPREMIUM_HW_STAT_CHART_H
#define GAMMARAYPREMIUM_HW_STAT_CHART_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QBrush>
#include <vector>

namespace tc
{

    enum HWStatChartType {
        kRegion,
        kLine,
    };

    class HWStatChart : public QWidget {
    public:
        explicit HWStatChart(QWidget* parent = nullptr);
        HWStatChart* SetRow(int row);
        HWStatChart* SetColumn(int column);
        HWStatChart* SetMaxValues(int count);
        HWStatChart* SetTitle(const QString& title);
        HWStatChart* SetYAxisDesc(const QString& desc);
        HWStatChart* SetChartType(const HWStatChartType& type);

        void paintEvent(QPaintEvent *event) override;
        void UpdateValues(const std::vector<float>& v);

    private:
        std::vector<float> values_;
        int row_ = 5;
        int column_ = 8;
        int max_values_ = 60;
        QString title_ = "Chart";
        QString y_axis_desc_;
        HWStatChartType type_ = HWStatChartType::kRegion;
    };

}

#endif //GAMMARAYPREMIUM_HW_STAT_CHART_H

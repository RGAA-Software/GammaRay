//
// Created by RGAA on 2024-04-20.
//

#ifndef GAMMARAY_STAT_CHART_H
#define GAMMARAY_STAT_CHART_H

#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QValueAxis>
#include <QCategoryAxis>
#include <QSplineSeries>
#include <QLineSeries>

#include <memory>
#include <map>

namespace tc
{

    class GrContext;

    class StatChart : public QWidget {
    public:

        explicit StatChart(const std::shared_ptr<GrContext>& ctx, const std::vector<QString>& line_names, QWidget* parent = nullptr);

        void UpdateLines(const std::map<QString, std::vector<uint32_t>>& value);

    private:

        std::shared_ptr<GrContext> ctx_ = nullptr;
        QChart* chart_ = nullptr;
        QChartView* chart_view_ = nullptr;
        QValueAxis* x_axis_ = nullptr;
        QValueAxis* y_axis_ = nullptr;
        std::map<QString, QLineSeries*> series_;

    };

}

#endif //GAMMARAY_STAT_CHART_H

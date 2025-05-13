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

    class ClientContext;

    class CtStatChartAxisSettings {
    public:
        int count_ = 0;
        float rng_beg_ = 0;
        float rng_end_ = 0;
        QString format_ = "";
    };

    class CtStatChart : public QWidget {
    public:
        explicit CtStatChart(const std::shared_ptr<ClientContext>& ctx,
                           const QString& title,
                           const std::vector<QString>& line_names,
                           const CtStatChartAxisSettings& x_axis_settings,
                           const CtStatChartAxisSettings& y_axis_settings,
                           QWidget* parent = nullptr);
        void UpdateTitle(const QString& title);
        void UpdateLines(const std::map<QString, std::vector<float>>& value);

    private:
        std::shared_ptr<ClientContext> ctx_ = nullptr;
        QChart* chart_ = nullptr;
        QChartView* chart_view_ = nullptr;
        QValueAxis* x_axis_ = nullptr;
        QValueAxis* y_axis_ = nullptr;
        std::map<QString, QLineSeries*> series_;

    };

}

#endif //GAMMARAY_STAT_CHART_H

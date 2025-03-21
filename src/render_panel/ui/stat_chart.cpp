//
// Created by RGAA on 2024-04-20.
//

#include "stat_chart.h"
#include "render_panel/gr_context.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_common_new/log.h"
#include "tc_common_new/time_ext.h"

#include <QTime>
#include <QTimer>
#include <QRandomGenerator>

namespace tc
{

    StatChart::StatChart(const std::shared_ptr<GrContext>& ctx, const std::vector<QString>& line_names, QWidget* parent) : QWidget(parent) {
        ctx_ = ctx;
        setStyleSheet("background-color:#ffffff; border: 1px solid #eeeeee;");
        auto layout = new NoMarginVLayout();
        chart_ = new QChart();
        for (auto& n : line_names) {
            auto s = new QLineSeries();
//            auto pen = s->pen();
//            pen.setWidth(2);
//            pen.setColor(pen.color());
//            s->setPen(pen);
            s->setName(n);
            series_.insert({n, s});
            chart_->addSeries(s);
        }

        chart_view_ = new QChartView(this);
        chart_view_->setRenderHint(QPainter::Antialiasing);
        chart_view_->setChart(chart_);
        layout->addWidget(chart_view_);

        x_axis_ = new QValueAxis();
        x_axis_->setTickCount(20);
        x_axis_->setRange(0, 180);
        x_axis_->setLabelFormat("%d");
        chart_->addAxis(x_axis_, Qt::AlignBottom);
        for (auto& [n, s] : series_) {
            s->attachAxis(x_axis_);
        }

        y_axis_ = new QValueAxis();
        y_axis_->setRange(0, 120);
        y_axis_->setLabelFormat("%d ms");
        chart_->addAxis(y_axis_, Qt::AlignLeft);
        for (auto& [n, s] : series_) {
            s->attachAxis(y_axis_);
        }

        setLayout(layout);
    }

    void StatChart::UpdateLines(const std::map<QString, std::vector<uint32_t>>& value) {
        ctx_->PostUITask([=, this]() {
            auto beg = TimeExt::GetCurrentTimestamp();
            for (auto& [in_n, in_v] : value) {
                for (auto& [n, s] : series_) {
                    if (in_n == n) {
                        QList<QPointF> points;
                        for (int i = 0; i < in_v.size(); i++) {
                            points.push_back(QPointF(i, in_v.at(i)));
                        }
                        s->clear();
                        s->replace(points);
                        break;
                    }
                }
            }
            chart_view_->update();
            auto end = TimeExt::GetCurrentTimestamp();
            //LOGI("UpdateLines UI, {}ms", (end-beg));
        });
    }

}
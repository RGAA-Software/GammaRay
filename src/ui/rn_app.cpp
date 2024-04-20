//
// Created by RGAA on 2024-04-11.
//

#include <QLabel>
#include <QTimer>
#include "rn_app.h"

#include "widgets/no_margin_layout.h"
#include "stat_chart.h"
#include "gr_statistics.h"
#include "tc_common_new/log.h"

constexpr auto kChartVideoFrameGap = "Video Frame Gap";
constexpr auto kChartEncode = "Encode";
constexpr auto kChartDecode = "Decode";

namespace tc
{

    RnApp::RnApp(const std::shared_ptr<GrContext>& ctx, QWidget *parent) : TabBase(ctx, parent) {
        auto root_layout = new NoMarginVLayout();
        auto place_holder = new QLabel();
        place_holder->setFixedWidth(1300);
        root_layout->addWidget(place_holder);

        {
            auto chart = new StatChart(ctx, {kChartVideoFrameGap, kChartEncode, kChartDecode}, this);
            stat_chart_ = chart;
            chart->setFixedSize(1100, 400);
            root_layout->addWidget(chart);
        }

        root_layout->addStretch();

        setLayout(root_layout);

        auto timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [=, this]() {
            this->UpdateUI();
        });
        timer->start(100);

    }

    void RnApp::OnTabShow() {

    }

    void RnApp::OnTabHide() {

    }

    void RnApp::UpdateUI() {
        auto stat = GrStatistics::Instance();
        std::map<QString, std::vector<uint32_t>> stat_value;
        // update video frame gap
        stat_value.insert({kChartVideoFrameGap, stat->video_frame_gaps_});
        // update encode durations
        stat_value.insert({kChartEncode, stat->encode_durations_});

        stat_chart_->UpdateLines(stat_value);
    }

}
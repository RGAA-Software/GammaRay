//
// Created by RGAA on 2024-04-11.
//

#include <QLabel>
#include <QTimer>
#include "rn_app.h"

#include "widgets/no_margin_layout.h"
#include "stat_chart.h"
#include "gr_statistics.h"
#include "gr_context.h"
#include "app_messages.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"

constexpr auto kChartVideoFrameGap = "Capture Video Gap";
constexpr auto kChartEncode = "Encode";
constexpr auto kChartDecode = "Decode";
constexpr auto kChartRecvVideoFrame = "Recv Video Frame";

namespace tc
{

    RnApp::RnApp(const std::shared_ptr<GrContext>& ctx, QWidget *parent) : TabBase(ctx, parent) {
        auto root_layout = new NoMarginVLayout();
        auto place_holder = new QLabel();
        place_holder->setFixedWidth(1300);
        root_layout->addWidget(place_holder);

        {
            auto chart = new StatChart(ctx, {kChartVideoFrameGap, kChartEncode, kChartDecode, kChartRecvVideoFrame}, this);
            stat_chart_ = chart;
            chart->setFixedSize(1100, 400);
            root_layout->addWidget(chart);
        }

        root_layout->addStretch();

        setLayout(root_layout);

        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgGrTimer100>([=, this](const MsgGrTimer100& msg) {
            this->UpdateUI();
        });

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
        // update decode durations
        stat_value.insert({kChartDecode, stat->decode_durations_});
        // update recv video frame gamp
        stat_value.insert({kChartRecvVideoFrame, stat->client_video_recv_gaps_});
        stat_chart_->UpdateLines(stat_value);
    }

}
//
// Created by RGAA on 2024-04-11.
//

#include "rn_app.h"

#include <QLabel>
#include <QTimer>
#include <QPushButton>

#include "widgets/no_margin_layout.h"
#include "stat_chart.h"
#include "gr_statistics.h"
#include "gr_context.h"
#include "app_messages.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include "widgets/round_img_display.h"

constexpr auto kChartVideoFrameGap = "Capture Video Gap";
constexpr auto kChartAudioFrameGap = "Capture Audio Gap";
constexpr auto kChartEncode = "Encode";
constexpr auto kChartDecode = "Decode";
constexpr auto kChartRecvVideoFrame = "Recv Video Gap";

namespace tc
{

    RnApp::RnApp(const std::shared_ptr<GrContext>& ctx, QWidget *parent) : TabBase(ctx, parent) {
        auto root_layout = new NoMarginVLayout();
        auto place_holder = new QLabel();
        place_holder->setFixedWidth(1300);
        root_layout->addWidget(place_holder);

        {
            auto head_layout = new NoMarginHLayout();
            // image
            {
                auto layout = new NoMarginVLayout();
                int item_width = 135;
                int item_height = item_width / (600.0 / 900.0);
                auto cover = new RoundImageDisplay("", item_width, item_height, 9, this);
                layout->addWidget(cover);
                layout->addStretch();
                head_layout->addSpacing(32);
                head_layout->addLayout(layout);
            }

            // app info
            auto label_size = QSize(140, 35);
            auto btn_height = 25;
            {
                auto layout = new NoMarginVLayout();
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("Running Game");
                    label->setStyleSheet("font-size: 14px;"); // background-color:#909090;
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    op->setText("Serious");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 14px;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("Stop Game");
                    label->setStyleSheet("font-size: 14px;");
                    item_layout->addWidget(label);

                    auto op = new QPushButton(this);
                    op->setText("STOP");
                    op->setFixedSize(QSize(70, btn_height));
                    op->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                layout->addStretch();
                head_layout->addSpacing(20);
                head_layout->addLayout(layout);
            }
            head_layout->addStretch();

            root_layout->addSpacing(15);
            root_layout->addLayout(head_layout);
        }

        {
            auto chart = new StatChart(ctx, {kChartVideoFrameGap, kChartAudioFrameGap, kChartEncode, kChartDecode, kChartRecvVideoFrame}, this);
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
        // update audio frame gap
        stat_value.insert({kChartAudioFrameGap, stat->audio_frame_gaps_});
        stat_chart_->UpdateLines(stat_value);
    }

}
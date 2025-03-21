//
// Created by RGAA on 2024-04-11.
//

#include "rn_app.h"

#include <QLabel>
#include <QTimer>
#include <QPushButton>

#include "tc_qt_widget/no_margin_layout.h"
#include "stat_chart.h"
#include "render_panel/gr_statistics.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_app_messages.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/num_formatter.h"
#include "render_panel/gr_run_game_manager.h"
#include "render_panel/db/db_game.h"

constexpr auto kChartVideoFrameGap = "Capture Video Gap";
constexpr auto kChartAudioFrameGap = "Capture Audio Gap";
constexpr auto kChartEncode = "Encode";
constexpr auto kChartDecode = "Decode";
constexpr auto kChartRecvVideoFrame = "Recv Video Gap";

namespace tc
{

    RnApp::RnApp(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginVLayout();
        auto place_holder = new QLabel();
        place_holder->setFixedWidth(1100);
        place_holder->setFixedHeight(1);
        root_layout->addWidget(place_holder);
        auto margin_left = 40;

        {
            auto head_layout = new NoMarginHLayout();
            head_layout->addSpacing(margin_left);
            // app info
            auto label_size = QSize(220, 35);

            // 2nd column
            {
                //
                auto layout = new NoMarginVLayout();
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("Renderer Running");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_app_running_time_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("FPS(Encode)");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_fps_encode_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("Sent Data");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_send_media_bytes_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("Capture Size");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_capture_size_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                //layout->addStretch();
                //head_layout->addSpacing(10);
                head_layout->addLayout(layout);
            }

            // 3rd column
            {
                //
                auto layout = new NoMarginVLayout();
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("FPS(Video Receive)");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_fps_video_recv_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("FPS(Client Render)");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_fps_render_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("Client Received Data");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_recv_media_data_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("Render Size");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_render_size_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                head_layout->addLayout(layout);
            }
            head_layout->addStretch();

            root_layout->addLayout(head_layout);
        }

        {
            auto layout = new NoMarginHLayout();
            layout->addSpacing(margin_left);
            auto chart = new StatChart(app_->GetContext(), {kChartVideoFrameGap, kChartAudioFrameGap, kChartEncode, kChartDecode, kChartRecvVideoFrame}, this);
            stat_chart_ = chart;
            chart->setFixedSize(680, 360);
            layout->addWidget(chart);
            layout->addStretch();
            root_layout->addSpacing(20);
            root_layout->addLayout(layout);
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

        lbl_fps_video_recv_->setText(std::to_string(stat->client_fps_video_recv_).c_str());
        lbl_fps_render_->setText(std::to_string(stat->client_fps_render_).c_str());
        lbl_recv_media_data_->setText(NumFormatter::FormatStorageSize(stat->client_recv_media_data_).c_str());

        lbl_app_running_time_->setText(NumFormatter::FormatTime(stat->app_running_time*1000).c_str());
        lbl_fps_encode_->setText(std::to_string(stat->fps_video_encode).c_str());
        lbl_send_media_bytes_->setText(NumFormatter::FormatStorageSize(stat->server_send_media_bytes).c_str());

        lbl_capture_size_->setText(std::format("{}x{}", stat->capture_width_, stat->capture_height_).c_str());
        lbl_render_size_->setText(std::format("{}x{}", stat->render_width_, stat->render_height_).c_str());

    }

}
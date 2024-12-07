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
#include "gr_application.h"
#include "app_messages.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/num_formatter.h"
#include "widgets/round_img_display.h"
#include "src/gr_run_game_manager.h"
#include "db/db_game.h"

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
        root_layout->addWidget(place_holder);

        {
            auto head_layout = new NoMarginHLayout();
            // app info
            auto label_size = QSize(220, 35);
            auto btn_height = 25;

            // 1st column
            {
                auto layout = new NoMarginVLayout();
                {
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("Running Games");
                    label->setStyleSheet("font-size: 13px;font-weight:700;");
                    layout->addWidget(label);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    lbl_running_games_ = label;
                    label->setFixedSize(label_size);
                    label->setText("");
                    label->setStyleSheet("font-size: 14px;");
                    item_layout->addWidget(label);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                layout->addStretch();
                head_layout->addSpacing(10);
                head_layout->addLayout(layout);
            }

            // 2nd column
            {
                //
                auto layout = new NoMarginVLayout();
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("App Running");
                    label->setStyleSheet("font-size: 13px;"); // background-color:#909090;
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_app_running_time_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("FPS(Encode)");
                    label->setStyleSheet("font-size: 13px;"); // background-color:#909090;
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_fps_encode_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("Sent Data");
                    label->setStyleSheet("font-size: 13px;"); // background-color:#909090;
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_send_media_bytes_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("Capture Size");
                    label->setStyleSheet("font-size: 13px;"); // background-color:#909090;
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_capture_size_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                layout->addStretch();
                head_layout->addSpacing(10);
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
                    label->setStyleSheet("font-size: 13px;"); // background-color:#909090;
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_fps_video_recv_ = op;
                    op->setText("");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new QLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("FPS(Client Render)");
                    label->setStyleSheet("font-size: 13px;"); // background-color:#909090;
                    item_layout->addWidget(label);

                    auto op = new QLabel(this);
                    lbl_fps_render_ = op;
                    op->setText("Serious");
                    op->setFixedSize(QSize(150, label_size.height()));
                    op->setStyleSheet("font-size: 13px;");
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
                    op->setStyleSheet("font-size: 13px;");
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
                    op->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                layout->addStretch();
                head_layout->addSpacing(10);
                head_layout->addLayout(layout);
            }
            head_layout->addStretch();

            root_layout->addSpacing(15);
            root_layout->addLayout(head_layout);
        }

        {
            auto chart = new StatChart(app_->GetContext(), {kChartVideoFrameGap, kChartAudioFrameGap, kChartEncode, kChartDecode, kChartRecvVideoFrame}, this);
            stat_chart_ = chart;
            chart->setFixedSize(900, 400);
            root_layout->addWidget(chart);
        }

        root_layout->addStretch();

        setLayout(root_layout);

        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgGrTimer100>([=, this](const MsgGrTimer100& msg) {
            this->UpdateUI();
        });

        msg_listener_->Listen<MsgRunningGameIds>([=, this](const MsgRunningGameIds& msg) {
            this->context_->PostUITask([=, this]() {
                auto rgm = this->context_->GetRunGameManager();
                auto running_games = rgm->GetRunningGames();
                std::string running_games_name;
                for (const auto& rg : running_games) {
                    running_games_name = running_games_name
                            .append(std::to_string(rg->game_->game_id_))
                            .append(" - ")
                            .append(rg->game_->game_name_).append("\n");
                }
                lbl_running_games_->setText(running_games_name.c_str());
            });
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
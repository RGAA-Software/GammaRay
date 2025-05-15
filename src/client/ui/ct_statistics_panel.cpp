//
// Created by RGAA on 12/08/2024.
//

#include "ct_statistics_panel.h"
#include "key_state_panel.h"
#include "client/ct_client_context.h"
#include "no_margin_layout.h"
#include "float_icon.h"
#include "ct_stat_chart.h"
#include "tc_client_sdk_new/sdk_statistics.h"
#include "tc_client_sdk_new/sdk_messages.h"
#include "tc_common_new/num_formatter.h"
#include "ct_stat_frame_info_item.h"
#include <QLabel>

namespace tc
{

    const QString kChartDataSpeed = "Video&Audio Data Speed";
    const QString kChartNetworkDelay = "Network Delay";
    const QString kChartDecodeDuration = "Decode Duration(ms)";
    const QString kChartVideoFrameGap = "Video Frame Gap(ms)";
    const QString kChartVideoFrameFps = "Video Frame FPS";

    CtStatisticsPanel::CtStatisticsPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        setWindowTitle("Statistics");

        sdk_stat_ = SdkStatistics::Instance();

        auto root_layout = new NoMarginHLayout();
        root_layout->addSpacing(20);
        // LEFT
        {
            auto layout = new NoMarginVLayout();
            root_layout->addLayout(layout);
            // Keyboard status
            {
                layout->addSpacing(20);
                auto lbl = new QLabel(this);
                lbl->setStyleSheet(R"(font-size: 16px; font-weight: bold;)");
                lbl->setText(tr("Server Keyboard Status"));
                layout->addWidget(lbl);
                layout->addSpacing(10);

                key_state_panel_ = new KeyStatePanel(ctx, this);
                layout->addWidget(key_state_panel_);
            }

            // global statistics info
            // data speed chart
            {
                {
                    layout->addSpacing(20);
                    auto lbl = new QLabel(this);
                    lbl_data_speed_ = lbl;
                    lbl->setStyleSheet(R"(font-size: 16px; font-weight: bold;)");
                    lbl->setText(tr("Media Data Speed"));
                    layout->addWidget(lbl);
                    layout->addSpacing(10);

                    data_speed_stat_chart_ = new CtStatChart(context_, "Data Speed", {
                            kChartDataSpeed,
                        }, CtStatChartAxisSettings {
                            .count_ = 15,
                            .rng_beg_ = 0,
                            .rng_end_ = 180,
                            .format_ = "%d"
                        }, CtStatChartAxisSettings {
                            .count_ = 6,
                            .rng_beg_ = 0,
                            .rng_end_ = 5,
                            .format_ = "%d MB/s"
                        }, this);
                    data_speed_stat_chart_->setFixedSize(600, 250);
                    layout->addWidget(data_speed_stat_chart_);
                }
            }

            // data speed chart
            {
                layout->addSpacing(20);
                auto lbl = new QLabel(this);
                lbl->setStyleSheet(R"(font-size: 16px; font-weight: bold;)");
                lbl->setText(tr("Network"));
                layout->addWidget(lbl);
                layout->addSpacing(10);

                durations_stat_chart_ = new CtStatChart(context_, "Network Information", {
                        kChartNetworkDelay,
                }, CtStatChartAxisSettings {
                        .count_ = 15,
                        .rng_beg_ = 0,
                        .rng_end_ = 180,
                        .format_ = "%d"
                }, CtStatChartAxisSettings {
                        .count_ = 4,
                        .rng_beg_ = 0,
                        .rng_end_ = 500,
                        .format_ = "%d ms"
                }, this);
                durations_stat_chart_->setFixedSize(600, 250);
                layout->addWidget(durations_stat_chart_);
                layout->addStretch();
            }
        }

        root_layout->addSpacing(20);

        // RIGHT
        {
            //auto margin_left = 20;
            auto right_layout = new NoMarginVLayout();
            {

                auto lbl = new QLabel(this);
                lbl->setStyleSheet(R"(font-size: 16px; font-weight: bold;)");
                lbl->setText(tr("Information"));
                right_layout->addSpacing(10);
                right_layout->addWidget(lbl);
                right_layout->addSpacing(10);

                auto label_size = QSize(150, 30);
                auto value_size = QSize(80, 30);

                // line 1
                {
                    //
                    auto layout = new NoMarginHLayout();
                    // received data
                    {
                        auto item_layout = new NoMarginHLayout();
                        auto label = new QLabel(this);
                        label->setFixedSize(label_size);
                        label->setText("Received Data");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_received_data_ = op;
                        op->setText("");
                        op->setFixedSize(value_size);
                        op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }

                    // video format
                    {
                        auto item_layout = new NoMarginHLayout();
                        auto label = new QLabel(this);
                        label->setFixedSize(label_size);
                        label->setText("Video Format");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_video_format_ = op;
                        op->setText("");
                        op->setFixedSize(value_size);
                        op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }

                    // video color
                    {
                        auto item_layout = new NoMarginHLayout();
                        auto label = new QLabel(this);
                        label->setFixedSize(label_size);
                        label->setText("Video Color Mode");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_video_color_ = op;
                        op->setText("");
                        op->setFixedSize(value_size);
                        op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }
                    right_layout->addLayout(layout);

                } // end line 1

                // line 2
                {
                    //
                    auto layout = new NoMarginHLayout();
                    // video decoder
                    {
                        auto item_layout = new NoMarginHLayout();
                        auto label = new QLabel(this);
                        label->setFixedSize(label_size);
                        label->setText("Video Decoder");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_video_decoder_ = op;
                        op->setText("");
                        op->setFixedSize(value_size);
                        op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }

                    // video capture type
                    {
                        auto item_layout = new NoMarginHLayout();
                        auto label = new QLabel(this);
                        label->setFixedSize(label_size);
                        label->setText("Video Capture Type");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_video_capture_type_ = op;
                        op->setText("");
                        op->setFixedSize(value_size);
                        op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }

                    // audio capture type
                    {
                        auto item_layout = new NoMarginHLayout();
                        auto label = new QLabel(this);
                        label->setFixedSize(label_size);
                        label->setText("Audio Capture Mode");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_audio_capture_type_ = op;
                        op->setText("");
                        op->setFixedSize(value_size);
                        op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }
                    right_layout->addLayout(layout);

                } // end line 2

                // line 3
                {
                    //
                    auto layout = new NoMarginHLayout();
                    // audio encode type
                    {
                        auto item_layout = new NoMarginHLayout();
                        auto label = new QLabel(this);
                        label->setFixedSize(label_size);
                        label->setText("Audio Encode Type");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_audio_encode_type_ = op;
                        op->setText("");
                        op->setFixedSize(value_size);
                        op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }

                    {
                        auto item_layout = new NoMarginHLayout();
                        auto label = new QLabel(this);
                        label->setFixedSize(label_size);
                        label->setText("");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        //lbl_capture_type_ = op;
                        op->setText("");
                        op->setFixedSize(value_size);
                        op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }

                    {
                        auto item_layout = new NoMarginHLayout();
                        auto label = new QLabel(this);
                        label->setFixedSize(label_size);
                        label->setText("");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        //lbl_audio_capture_type_ = op;
                        op->setText("");
                        op->setFixedSize(value_size);
                        op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }
                    right_layout->addLayout(layout);

                } // end line 3

                // line 4
                {
                    //
                    auto layout = new NoMarginHLayout();
                    // remote computer info
                    {
                        auto item_layout = new NoMarginHLayout();
                        auto label = new QLabel(this);
                        label->setFixedSize(label_size);
                        label->setText("Remote PC Info");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_remote_computer_info_ = op;
                        op->setText("");
                        op->setFixedSize(value_size);
                        op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }
                    right_layout->addLayout(layout);

                } // end line 4

                // line 4
                {
                    //
                    auto layout = new NoMarginHLayout();
                    // remote computer info
                    {
                        auto item_layout = new NoMarginHLayout();
                        auto label = new QLabel(this);
                        label->setFixedSize(label_size);
                        label->setText("Local PC Info");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_local_computer_info_ = op;
                        op->setText("");
                        op->setFixedSize(value_size);
                        op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }
                    right_layout->addLayout(layout);

                } // end line 4

                // monitor info
                {

                    // title
                    auto lbl = new QLabel(this);
                    lbl->setStyleSheet(R"(font-size: 16px; font-weight: bold;)");
                    lbl->setText(tr("Isolated Monitor Information"));
                    right_layout->addSpacing(10);
                    right_layout->addWidget(lbl);
                    right_layout->addSpacing(6);

                    // chart
                    int capture_size = 4;
                    {
                        auto layout = new NoMarginHLayout();
                        for (int i = 0; i < capture_size; i++) {
                            auto item_widget = new CtStatFrameInfoItem(context_, this);
                            item_widget->setFixedSize(162, 185);

                            auto object_name = std::format("item{}", i);
                            item_widget->setObjectName(object_name.c_str());
                            item_widget->SetOnClickWidgetCallback([=, this](QWidget* w) {
                                for (const auto item : frame_info_items_) {
                                    if (item->objectName() == QString::fromStdString(object_name)) {
                                        item->Select();

                                        stat_chat_stack_->setCurrentIndex(i);
                                    }
                                    else {
                                        item->Unselect();
                                    }
                                }
                            });
                            layout->addWidget(item_widget);
                            layout->addSpacing(10);
                            frame_info_items_.push_back(item_widget);
                        }
                        layout->addStretch();
                        right_layout->addSpacing(10);
                        right_layout->addLayout(layout);
                    }

                    {
                        auto layout = new NoMarginHLayout();
                        stat_chat_stack_ = new QStackedWidget(this);
                        stat_chat_stack_->setFixedSize(680, 250);

                        for (int i = 0; i < capture_size; i++) {
                            auto chart = new CtStatChart(context_, "", {
                                    kChartVideoFrameGap,
                                    kChartDecodeDuration,
                                    kChartVideoFrameFps,
                            }, CtStatChartAxisSettings {
                                    .count_ = 15,
                                    .rng_beg_ = 0,
                                    .rng_end_ = 180,
                                    .format_ = "%d"
                            }, CtStatChartAxisSettings {
                                    .count_ = 4,
                                    .rng_beg_ = 0,
                                    .rng_end_ = 200,
                                    .format_ = "%d"
                            }, this);
                            chart->setFixedSize(stat_chat_stack_->size());
                            stat_charts_.push_back(chart);
                            stat_chat_stack_->addWidget(chart);
                        }

                        layout->addWidget(stat_chat_stack_);
                        layout->addStretch();
                        right_layout->addSpacing(10);
                        right_layout->addLayout(layout);
                    }
                }
            }

            right_layout->addStretch(100);
            root_layout->addLayout(right_layout);
        }
        root_layout->addStretch();
        root_layout->addSpacing(20);
        setLayout(root_layout);

        // select default page
        frame_info_items_[0]->Select();
        stat_chat_stack_->setCurrentIndex(0);

        // messages
        msg_listener_->Listen<SdkMsgTimer1000>([=, this](const SdkMsgTimer1000& msg) {
            context_->PostUITask([=, this]() {
                this->UpdateDataSpeedChart();
            });
        });
    }

    void CtStatisticsPanel::resizeEvent(QResizeEvent *event) {
        BaseWidget::resizeEvent(event);
    }

    void CtStatisticsPanel::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);
        painter.setBrush(QBrush(0xffffff));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(this->rect(), 7, 7);
    }

    void CtStatisticsPanel::UpdateOnHeartBeat(const OnHeartBeat& hb) {
        context_->PostUITask([=, this]() {
            key_state_panel_->alt_item_->UpdateState(hb.alt_pressed());
            key_state_panel_->shift_item_->UpdateState(hb.shift_pressed());
            key_state_panel_->control_item_->UpdateState(hb.control_pressed());
            key_state_panel_->win_item_->UpdateState(hb.win_pressed());
            key_state_panel_->caps_lock_item_->UpdateState(hb.caps_lock_pressed());
            key_state_panel_->num_lock_item_->UpdateState(hb.num_lock_pressed());
            key_state_panel_->caps_lock_item_->UpdateIndicator(hb.caps_lock_state()==1);
            key_state_panel_->num_lock_item_->UpdateIndicator(hb.num_lock_state()==1);
        });
    }

    void CtStatisticsPanel::UpdateDataSpeedChart() {
        if (!lbl_received_data_) {
            return;
        }
        // data speed
        {
            std::map<QString, std::vector<float>> stat_value;
            stat_value.insert({kChartDataSpeed, sdk_stat_->data_speeds_});
            data_speed_stat_chart_->UpdateLines(stat_value);
            if (!sdk_stat_->data_speeds_.empty()) {
                auto value = sdk_stat_->data_speeds_[sdk_stat_->data_speeds_.size() - 1] * 1024 * 1024;
                lbl_data_speed_->setText(std::format("Media Data Speed({})", NumFormatter::FormatSpeed(value)).c_str());
            }
        }

        //
        {
            std::map<QString, std::vector<float>> stat_value;
            stat_value.insert({kChartNetworkDelay, sdk_stat_->net_delays_});
            durations_stat_chart_->UpdateLines(stat_value);
        }

        {
            lbl_received_data_->setText(NumFormatter::FormatStorageSize(sdk_stat_->recv_data_).c_str());
            lbl_video_format_->setText(sdk_stat_->video_format_.c_str());
            lbl_video_color_->setText(sdk_stat_->video_color_.c_str());

            lbl_video_decoder_->setText(sdk_stat_->video_decoder_.c_str());
            lbl_video_capture_type_->setText(sdk_stat_->video_capture_type_.c_str());
            lbl_audio_capture_type_->setText(sdk_stat_->audio_capture_type_.c_str());

            lbl_audio_encode_type_->setText(sdk_stat_->audio_encode_type_.c_str());
        }

        {
            int index = 0;
            for (const auto& [mon_name, value] : sdk_stat_->decode_durations_) {
                if (index >= 4) {
                    break;
                }

                if (sdk_stat_->frames_size_.contains(mon_name) && sdk_stat_->render_monitor_stat_.contains(mon_name)) {
                    auto& frame_size = sdk_stat_->frames_size_[mon_name];
                    float target_recv_fps = 0;
                    if (sdk_stat_->video_recv_fps_.contains(mon_name)) {
                        if (auto& recv_fps = sdk_stat_->video_recv_fps_[mon_name]; !recv_fps.empty()) {
                            target_recv_fps = recv_fps[recv_fps.size()-1];
                        }
                    }
                    auto render_monitor_stat = sdk_stat_->render_monitor_stat_[mon_name];
                    frame_info_items_[index]->UpdateInfo(CtStatItemInfo{
                        .name_ = mon_name,
                        .frame_width_ = frame_size.width_,
                        .frame_height_ = frame_size.height_,
                        .received_fps_ = (int)target_recv_fps,
                        .render_capture_fps_ = render_monitor_stat.capture_fps(),
                        .render_capture_frame_width_ = render_monitor_stat.capture_frame_width(),
                        .render_capture_frame_height_ = render_monitor_stat.capture_frame_height(),
                        .render_encoder_name_ = render_monitor_stat.encoder_name(),
                        .render_encode_fps_ = render_monitor_stat.encode_fps(),
                    });
                }
                std::map<QString, std::vector<float>> stat_value;
                // decode duration
                stat_value.insert({kChartDecodeDuration, value});

                // gaps
                if (sdk_stat_->video_recv_gaps_.contains(mon_name)) {
                    stat_value.insert({kChartVideoFrameGap, sdk_stat_->video_recv_gaps_[mon_name]});
                }

                // video receive fps
                if (sdk_stat_->video_recv_fps_.contains(mon_name)) {
                    stat_value.insert({kChartVideoFrameFps, sdk_stat_->video_recv_fps_[mon_name]});
                }

                // update
                stat_charts_[index]->UpdateTitle(mon_name.c_str());
                stat_charts_[index]->UpdateLines(stat_value);

                index++;
            }

            for (int i = index; i < 4; i++) {
                frame_info_items_[index]->ClearInfo();
            }
        }
    }
}
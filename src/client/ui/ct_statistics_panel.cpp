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
#include "tc_common_new/hardware.h"
#include "ct_stat_frame_info_item.h"
#include "tc_qt_widget/widget_helper.h"
#include "client/ct_settings.h"
#include "tc_label.h"

namespace tc
{

    const QString kChartDataSpeed = "Video&Audio Data Speed";
    const QString kChartNetworkDelay = "Network Delay";
    const QString kChartDecodeDuration = "Decode Duration(ms)";
    const QString kChartVideoFrameGap = "Video Frame Gap(ms)";
    const QString kChartVideoFrameFps = "Video Frame FPS";

    CtStatisticsPanel::CtStatisticsPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        setWindowTitle(tcTr("id_statistics"));
        installEventFilter(this);
        sdk_stat_ = SdkStatistics::Instance();
        settings_ = Settings::Instance();

        auto root_layout = new NoMarginHLayout();
        root_layout->addSpacing(20);
        // LEFT
        {
            auto layout = new NoMarginVLayout();
            root_layout->addLayout(layout);
            // Keyboard status
            {
                layout->addSpacing(20);
                auto lbl = new TcLabel(this);
                lbl->setStyleSheet(R"(font-size: 16px; font-weight: bold;)");
                lbl->SetTextId("id_server_keyboard_status");
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
                    auto lbl = new TcLabel(this);
                    lbl_data_speed_ = lbl;
                    lbl->setStyleSheet(R"(font-size: 16px; font-weight: bold;)");
                    lbl->SetTextId("id_media_data_speed");
                    layout->addWidget(lbl);
                    layout->addSpacing(10);

                    data_speed_stat_chart_ = new CtStatChart(context_, tcTr("id_data_speed"), {
                            kChartDataSpeed,
                        }, CtStatChartAxisSettings {
                            .count_ = 15,
                            .rng_beg_ = 0,
                            .rng_end_ = 180,
                            .format_ = "%d"
                        }, CtStatChartAxisSettings {
                            .count_ = 7,
                            .rng_beg_ = 0,
                            .rng_end_ = 50,
                            .format_ = "%d MB/s"
                        }, this);
                    data_speed_stat_chart_->setFixedSize(600, 250);
                    layout->addWidget(data_speed_stat_chart_);
                }
            }

            // data speed chart
            {
                layout->addSpacing(20);
                auto lbl = new TcLabel(this);
                lbl->setStyleSheet(R"(font-size: 16px; font-weight: bold;)");
                lbl->SetTextId("id_network");
                layout->addWidget(lbl);
                layout->addSpacing(10);

                durations_stat_chart_ = new CtStatChart(context_, tcTr("id_network_info"), {
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

                auto lbl = new TcLabel(this);
                lbl->setStyleSheet(R"(font-size: 16px; font-weight: bold;)");
                lbl->SetTextId("id_info");
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
                        auto label = new TcLabel(this);
                        label->setFixedSize(label_size);
                        label->SetTextId("id_received_data");
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
                        auto label = new TcLabel(this);
                        label->setFixedSize(label_size);
                        label->SetTextId("id_video_format");
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
                        auto label = new TcLabel(this);
                        label->setFixedSize(label_size);
                        label->SetTextId("id_video_color_mode");
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
                        auto label = new TcLabel(this);
                        label->setFixedSize(label_size);
                        label->SetTextId("id_video_decoder");
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
                        auto label = new TcLabel(this);
                        label->setFixedSize(label_size);
                        label->SetTextId("id_video_capture_type");
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
                        auto label = new TcLabel(this);
                        label->setFixedSize(label_size);
                        label->SetTextId("id_audio_capture_type");
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
                        auto label = new TcLabel(this);
                        label->setFixedSize(label_size);
                        label->SetTextId("id_audio_encode_type");
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
                        auto label = new TcLabel(this);
                        label->setFixedSize(label_size);
                        label->SetTextId("id_connection_type");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_conn_type_ = op;
                        op->setText([=, this]() -> QString {
                            if (settings_->network_type_ == ClientNetworkType::kRelay) {
                                return "Relay";
                            }
                            else if (settings_->network_type_ == ClientNetworkType::kWebsocket) {
                                return "WSS";
                            }
                            else if (settings_->network_type_ == ClientNetworkType::kWebRtc) {
                                return "RTC";
                            }
                            else if (settings_->network_type_ == ClientNetworkType::kUdpKcp) {
                                return "UDP";
                            }
                            else {
                                return "";
                            }
                         }());
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
                        auto label = new TcLabel(this);
                        label->setFixedSize(label_size);
                        label->SetTextId("id_remote_pc_info");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_remote_computer_info_ = op;
                        op->setText("");
                        op->setFixedSize(QSize(520, 30));
                        op->setStyleSheet("font-size: 12px; font-weight:500; color: #2979ff;");
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
                        auto label = new TcLabel(this);
                        label->setFixedSize(label_size);
                        label->SetTextId("id_local_pc_info");
                        label->setStyleSheet("font-size: 13px;");
                        item_layout->addWidget(label);

                        auto op = new QLabel(this);
                        lbl_local_computer_info_ = op;
                        op->setText("");
                        op->setFixedSize(QSize(520, 30));
                        op->setStyleSheet("font-size: 12px; font-weight:500; color: #2979ff;");
                        item_layout->addWidget(op);
                        item_layout->addStretch();
                        layout->addLayout(item_layout);
                    }
                    right_layout->addLayout(layout);

                } // end line 4

                // monitor info
                {

                    // title
                    auto lbl = new TcLabel(this);
                    lbl->setStyleSheet(R"(font-size: 16px; font-weight: bold;)");
                    lbl->SetTextId("id_isolated_monitor_info");
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

    bool CtStatisticsPanel::eventFilter(QObject* object, QEvent* event) {
        if (event->type() == QEvent::Show && object == this) {
            WidgetHelper::SetTitleBarColor(this);
        }
        return QObject::eventFilter(object, event);
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
                lbl_data_speed_->setText(tcTr("id_media_data_speed") + std::format("({})", NumFormatter::FormatSpeed(value)).c_str());
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

            auto hardware = Hardware::Instance();
            std::stringstream ss;
            if (hardware->gpus_.empty()) {
                ss << "NO GPU";
            }
            else {
                for (const auto &gpu: hardware->gpus_) {
                    ss << gpu.name_ << ";";
                }
            }
            auto cpu_name = QString::fromStdString(hardware->hw_cpu_.name_).trimmed();
            lbl_local_computer_info_->setText(std::format("{} / {} / {}", cpu_name.toStdString(), NumFormatter::FormatStorageSize(hardware->memory_size_), ss.str()).c_str());
            lbl_local_computer_info_->setToolTip(lbl_local_computer_info_->text());

            lbl_remote_computer_info_->setText(sdk_stat_->remote_pc_info_.c_str());
            lbl_remote_computer_info_->setToolTip(lbl_remote_computer_info_->text());

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
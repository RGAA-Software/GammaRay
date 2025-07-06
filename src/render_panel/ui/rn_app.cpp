//
// Created by RGAA on 2024-04-11.
//

#include "rn_app.h"
#include <QTimer>
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
#include "render_panel/database/db_game.h"
#include "stat_capture_info_item.h"
#include "tc_label.h"
#include "tc_pushbutton.h"

constexpr auto kChartVideoFrameGap = "Capture Video Gap";
constexpr auto kChartAudioFrameGap = "Capture Audio Gap";
constexpr auto kChartEncode = "Encode";
constexpr auto kChartCopyTexture = "Copy&Resize Texture";
constexpr auto kChartMapCvtTexture = "Map&Cvt Texture";

namespace tc
{

    RnApp::RnApp(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginVLayout();
        auto place_holder = new TcLabel();
        place_holder->setFixedWidth(1100);
        place_holder->setFixedHeight(1);
        root_layout->addWidget(place_holder);
        auto margin_left = 30;

        {
            auto head_layout = new NoMarginHLayout();
            head_layout->addSpacing(margin_left);
            // app info
            auto label_size = QSize(150, 30);
            auto value_size = QSize(80, 30);

            // 2nd column
            {
                //
                auto layout = new NoMarginVLayout();
                // Running time
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new TcLabel(this);
                    label->setFixedSize(label_size);
                    label->SetTextId("id_renderer_running");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new TcLabel(this);
                    lbl_app_running_time_ = op;
                    op->setText("");
                    op->setFixedSize(value_size);
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }

                // Send media bytes
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new TcLabel(this);
                    label->setFixedSize(label_size);
                    label->SetTextId("id_sent_data");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new TcLabel(this);
                    lbl_send_media_bytes_ = op;
                    op->setText("");
                    op->setFixedSize(value_size);
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }

                // Send media speed
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new TcLabel(this);
                    label->setFixedSize(label_size);
                    label->SetTextId("id_send_speed");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new TcLabel(this);
                    lbl_send_media_speed_ = op;
                    op->setText("");
                    op->setFixedSize(value_size);
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
                auto layout = new NoMarginVLayout();
                // connected clients
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new TcLabel(this);
                    label->setFixedSize(label_size);
                    label->SetTextId("id_connected_clients");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new TcLabel(this);
                    lbl_connected_clients_ = op;
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

                    auto op = new TcLabel(this);
                    lbl_video_capture_type_ = op;
                    op->setText("");
                    op->setFixedSize(value_size);
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }

                // video encode type
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new TcLabel(this);
                    label->setFixedSize(label_size);
                    label->SetTextId("id_video_encode_type");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new TcLabel(this);
                    lbl_video_encode_type_ = op;
                    op->setText("");
                    op->setFixedSize(value_size);
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }

                //layout->addStretch();
                head_layout->addLayout(layout);
            }

            //
            {
                auto layout = new NoMarginVLayout();
                // relay connected
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new TcLabel(this);
                    label->setFixedSize(label_size);
                    label->SetTextId("id_relay_connected");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new TcLabel(this);
                    lbl_relay_connected_ = op;
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

                    auto op = new TcLabel(this);
                    lbl_audio_capture_type_ = op;
                    op->setText("");
                    op->setFixedSize(value_size);
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }

                // audio encode type
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new TcLabel(this);
                    label->setFixedSize(label_size);
                    label->SetTextId("id_audio_encode_type");
                    label->setStyleSheet("font-size: 13px;");
                    item_layout->addWidget(label);

                    auto op = new TcLabel(this);
                    lbl_audio_encode_type_ = op;
                    op->setText("");
                    op->setFixedSize(value_size);
                    op->setStyleSheet("font-size: 13px; font-weight:500; color: #2979ff;");
                    item_layout->addWidget(op);
                    item_layout->addStretch();
                    layout->addLayout(item_layout);
                }
                //layout->addStretch();
                head_layout->addLayout(layout);
            }
            head_layout->addStretch();

            root_layout->addLayout(head_layout);
        }

        // capturing info
        int capture_size = 4;
        {
            auto layout = new NoMarginHLayout();
            layout->addSpacing(margin_left);
            for (int i = 0; i < capture_size; i++) {
                auto item_widget = MakeCapturesInfoWidget();
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
            root_layout->addSpacing(10);
            root_layout->addLayout(layout);
        }

        {
            auto layout = new NoMarginHLayout();
            layout->addSpacing(margin_left);
            stat_chat_stack_ = new QStackedWidget(this);
            stat_chat_stack_->setFixedSize(680, 300);

            for (int i = 0; i < capture_size; i++) {
                auto chart = new StatChart(app_->GetContext(), "",
                                           { kChartVideoFrameGap,
                                             kChartAudioFrameGap,
                                             kChartEncode,
                                             kChartCopyTexture,
                                             kChartMapCvtTexture
                                             },
                                           this);
                chart->setFixedSize(stat_chat_stack_->size());
                stat_charts_.push_back(chart);
                stat_chat_stack_->addWidget(chart);
            }

            layout->addWidget(stat_chat_stack_);
            layout->addStretch();
            root_layout->addSpacing(20);
            root_layout->addLayout(layout);
        }
        root_layout->addStretch();
        root_layout->addSpacing(20);

        setLayout(root_layout);

        // select default chart
        frame_info_items_[0]->Select();
        stat_chat_stack_->setCurrentIndex(0);

        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgGrTimer1S>([=, this](const MsgGrTimer1S& msg) {
            this->UpdateUI();
        });
    }

    void RnApp::OnTabShow() {

    }

    void RnApp::OnTabHide() {

    }

    void RnApp::UpdateUI() {
        auto stat = GrStatistics::Instance();

        // column 1
        lbl_app_running_time_->setText(NumFormatter::FormatTime(stat->app_running_time*1000).c_str());
        lbl_send_media_bytes_->setText(NumFormatter::FormatStorageSize(stat->server_send_media_bytes).c_str());
        lbl_send_media_speed_->setText(NumFormatter::FormatSpeed(stat->send_speed_bytes).c_str());

        // column 2
        lbl_connected_clients_->setText(std::to_string(stat->connected_clients_).c_str());
        lbl_video_capture_type_->setText(stat->video_capture_type_.Load().c_str());
        lbl_video_encode_type_->setText(stat->video_encode_type_.Load().c_str());

        // column 3
        lbl_relay_connected_->setText(stat->relay_connected_ ? "Online" : "Offline");
        lbl_audio_capture_type_->setText(stat->audio_capture_type_.Load().c_str());
        lbl_audio_encode_type_->setText("OPUS");

//        for (const auto& cp : frame_info_items_) {
//            cp->ClearInfo();
//        }
        int index = 0;
        auto captures_info = stat->GetCapturesInfo();
        for (const auto& info : captures_info) {
            if (index >= 4) {
                break;
            }
            stat_charts_[index]->UpdateTitle(info->target_name().c_str());
            frame_info_items_[index]->UpdateInfo(info);

            std::map<QString, std::vector<int32_t>> stat_value;
            // update video frame gap
            auto video_capture_gaps = stat->GetVideoCaptureGaps();
            if (video_capture_gaps.contains(info->target_name())) {
                stat_value.insert({kChartVideoFrameGap, video_capture_gaps[info->target_name()]});
            }

            // update encode durations
            auto encode_durations = stat->GetEncodeDurations();
            if (encode_durations.contains(info->target_name())) {
                stat_value.insert({kChartEncode, encode_durations[info->target_name()]});
            }

            // copy / resize texture
            auto copy_texture_durations = stat->GetCopyTextureDurations();
            if (copy_texture_durations.contains(info->target_name())) {
                stat_value.insert({kChartCopyTexture, copy_texture_durations[info->target_name()]});
            }

            auto map_cvt_texture_durations = stat->GetMapCvtTextureDurations();
            if (map_cvt_texture_durations.contains(info->target_name())) {
                stat_value.insert({kChartMapCvtTexture, map_cvt_texture_durations[info->target_name()]});
            }

            if (index == 0) {
                // update audio frame gap
                auto audio_frame_gaps = stat->GetAudioFrameGaps();
                stat_value.insert({kChartAudioFrameGap, audio_frame_gaps});
            }
            stat_charts_[index]->UpdateLines(stat_value);

            index++;
        }
        for (int i = index; i < 4; i++) {
            frame_info_items_[i]->ClearInfo();
        }

    }

    StatCaptureInfoItem* RnApp::MakeCapturesInfoWidget() {
        auto item = new StatCaptureInfoItem(context_, this);
        item->setFixedSize(162, 185);
        return item;
    }

}
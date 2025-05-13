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
#include <QLabel>

namespace tc
{

    const QString kChartDataSpeed = "Video&Audio Data Speed";
    const QString kChartNetworkDelay = "Network Delay";

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
                            .count_ = 4,
                            .rng_beg_ = 0,
                            .rng_end_ = 3,
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
                lbl->setText(tr("Time Durations"));
                layout->addWidget(lbl);
                layout->addSpacing(10);

                durations_stat_chart_ = new CtStatChart(context_, "Durations", {
                        kChartDataSpeed,
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

        // RIGHT
        {

        }
        root_layout->addStretch();
        root_layout->addSpacing(20);
        setLayout(root_layout);

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
            //stat_value.insert({kChartNetworkDelay, sdk_stat_->data_speeds_});
            //durations_stat_chart_->UpdateLines(stat_value);
        }
    }
}
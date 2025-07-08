//
// Created by RGAA on 12/08/2024.
//

#ifndef GAMMARAYPC_CT_STATISTICS_PANEL_H
#define GAMMARAYPC_CT_STATISTICS_PANEL_H

#include "base_widget.h"
#include "tc_message.pb.h"
#include <QLabel>
#include <QStackedWidget>

namespace tc
{

    class KeyStatePanel;
    class FloatIcon;
    class CtStatChart;
    class SdkStatistics;
    class CtStatFrameInfoItem;
    class Settings;
    class Message;

    class CtStatisticsPanel : public BaseWidget {
    public:
        explicit CtStatisticsPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);

        void resizeEvent(QResizeEvent *event) override;
        void paintEvent(QPaintEvent *event) override;
        bool eventFilter(QObject* object, QEvent* event) override;
        void UpdateOnHeartBeat(std::shared_ptr<tc::Message> msg);
    private:
        void UpdateDataSpeedChart();

    private:
        KeyStatePanel* key_state_panel_ = nullptr;
        FloatIcon* close_btn_ = nullptr;
        CtStatChart* data_speed_stat_chart_ = nullptr;
        CtStatChart* durations_stat_chart_ = nullptr;
        SdkStatistics* sdk_stat_ = nullptr;
        Settings* settings_ = nullptr;
        QLabel* lbl_data_speed_ = nullptr;
        QLabel* lbl_received_data_ = nullptr;
        QLabel* lbl_sent_data_ = nullptr;
        QLabel* lbl_video_format_ = nullptr;
        QLabel* lbl_video_color_ = nullptr;
        QLabel* lbl_video_capture_type_ = nullptr;
        QLabel* lbl_audio_capture_type_ = nullptr;
        QLabel* lbl_audio_encode_type_ = nullptr;
        QLabel* lbl_conn_type_ = nullptr;
        QLabel* lbl_remote_computer_info_ = nullptr;
        QLabel* lbl_local_computer_info_ = nullptr;
        QLabel* lbl_network_ = nullptr;

        QLabel* lbl_video_decoder_ = nullptr;
        std::vector<CtStatFrameInfoItem*> frame_info_items_;
        QStackedWidget* stat_chat_stack_ = nullptr;
        std::vector<CtStatChart*> stat_charts_;
    };

}

#endif //GAMMARAYPC_DEBUG_PANEL_H

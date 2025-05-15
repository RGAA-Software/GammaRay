//
// Created by RGAA on 2024-04-11.
//

#ifndef TC_SERVER_STEAM_RNST_APP_H
#define TC_SERVER_STEAM_RNST_APP_H

#include "tab_base.h"
#include <QLabel>
#include <QStackedWidget>

namespace tc
{
    class StatChart;
    class MessageListener;
    class StatCaptureInfoItem;

    class RnApp : public TabBase {
    public:
        RnApp(const std::shared_ptr<GrApplication>& app, QWidget *parent);
        ~RnApp() override = default;

        void OnTabShow() override;
        void OnTabHide() override;

        void UpdateUI();

    private:
        StatCaptureInfoItem* MakeCapturesInfoWidget();

    private:
        std::shared_ptr<MessageListener> msg_listener_;

        QLabel* lbl_connected_clients_ = nullptr;
        QLabel* lbl_video_capture_type_ = nullptr;
        QLabel* lbl_video_encode_type_ = nullptr;
        QLabel* lbl_audio_encode_type_ = nullptr;
        QLabel* lbl_relay_connected_ = nullptr;
        QLabel* lbl_audio_capture_type_ = nullptr;

        QLabel* lbl_app_running_time_ = nullptr;
        QLabel* lbl_send_media_bytes_ = nullptr;
        QLabel* lbl_send_media_speed_ = nullptr;

        std::vector<StatCaptureInfoItem*> frame_info_items_;
        QStackedWidget* stat_chat_stack_ = nullptr;
        std::vector<StatChart*> stat_charts_;
    };

}

#endif //TC_SERVER_STEAM_ST_INPUT_H

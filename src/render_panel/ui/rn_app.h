//
// Created by RGAA on 2024-04-11.
//

#ifndef TC_SERVER_STEAM_RNST_APP_H
#define TC_SERVER_STEAM_RNST_APP_H

#include "tab_base.h"
#include <QLabel>

namespace tc
{
    class StatChart;
    class MessageListener;

    class RnApp : public TabBase {
    public:
        RnApp(const std::shared_ptr<GrApplication>& app, QWidget *parent);
        ~RnApp() = default;

        void OnTabShow() override;
        void OnTabHide() override;

        void UpdateUI();

    private:
        StatChart* stat_chart_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_;

        QLabel* lbl_connected_clients_ = nullptr;
        QLabel* lbl_capture_target_ = nullptr;
        QLabel* lbl_capture_fps_ = nullptr;
        //QLabel* lbl_render_size_ = nullptr;

        QLabel* lbl_app_running_time_ = nullptr;
        QLabel* lbl_fps_encode_ = nullptr;
        QLabel* lbl_send_media_bytes_ = nullptr;

        QLabel* lbl_capture_size_ = nullptr;
    };

}

#endif //TC_SERVER_STEAM_ST_INPUT_H

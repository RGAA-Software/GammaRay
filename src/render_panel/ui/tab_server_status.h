//
// Created by RGAA on 4/02/2025.
//

#ifndef GAMMARAY_TAB_SERVER_STATUS_H
#define GAMMARAY_TAB_SERVER_STATUS_H

#include "tab_base.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QStackedWidget>
#include <QLabel>

namespace tc
{

    class RnApp;
    class RnEmpty;
    class MessageListener;
    class QtCircle;
    class QtVertical;
    class GrStatistics;

    class TabServerStatus : public TabBase {
    public:
        explicit TabServerStatus(const std::shared_ptr<GrApplication>& app, QWidget *parent);
        ~TabServerStatus() override;

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        void RestartServer();
        void RefreshVigemState(bool ok);
        void RefreshServerState(bool ok);
        void RefreshServiceState(bool ok);
        void RefreshIndicatorState(QLabel* indicator, bool ok);
        void RefreshUIEverySecond();

    private:
        QLabel* lbl_vigem_state_ = nullptr;
        QLabel* lbl_renderer_state_ = nullptr;
        QLabel* lbl_service_state_ = nullptr;
        QLabel* lbl_audio_format_ = nullptr;
        QLabel* lbl_running_games_ = nullptr;
        //QtCircle* spectrum_circle_ = nullptr;
        QtVertical* spectrum_vertical_ = nullptr;
        QStackedWidget* rn_stack_ = nullptr;
        RnApp* rn_app_ = nullptr;

    };

}

#endif //GAMMARAY_TAB_SERVER_STATUS_H

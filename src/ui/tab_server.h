//
// Created by RGAA on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_TAB_SERVER_H
#define TC_SERVER_STEAM_TAB_SERVER_H

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
    class GrStatistics;

    class TabServer : public TabBase {
    public:
        explicit TabServer(const std::shared_ptr<GrApplication>& app, QWidget *parent);
        ~TabServer() override;

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        QString GetItemIconStyleSheet(const QString& url);
        void RefreshVigemState(bool ok);
        void RefreshServerState(bool ok);
        void RefreshIndicatorState(QLabel* indicator, bool ok);
        void RefreshUIEverySecond();
        void RestartServer();

    private:
        GrStatistics* statistics_ = nullptr;
        QPixmap qr_pixmap_;
        QStackedWidget* rn_stack_ = nullptr;
        RnApp* rn_app_ = nullptr;
        //RnEmpty* rn_empty_ = nullptr;
        QLabel* lbl_vigem_state_ = nullptr;
        QLabel* lbl_server_state_ = nullptr;
        QLabel* lbl_audio_format_ = nullptr;
        QtCircle* spectrum_circle_ = nullptr;

        std::shared_ptr<MessageListener> msg_listener_ = nullptr;

    };
}

#endif //TC_SERVER_STEAM_TAB_SERVER_H

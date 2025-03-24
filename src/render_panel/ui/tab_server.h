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
    class GrSettings;
    class TcQRWidget;
    class StreamContent;

    class TabServer : public TabBase {
    public:
        explicit TabServer(const std::shared_ptr<GrApplication>& app, QWidget *parent);
        ~TabServer() override;
        void OnTabShow() override;
        void OnTabHide() override;
        void RegisterMessageListener();

    private:
        void UpdateQRCode();

    private:
        GrSettings* settings_ = nullptr;
        QPixmap qr_pixmap_;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        QLabel* lbl_machine_code_ = nullptr;
        QLabel* lbl_machine_random_pwd_ = nullptr;
        TcQRWidget* lbl_qr_code_ = nullptr;

        // client
        StreamContent* stream_content_ = nullptr;

    };
}

#endif //TC_SERVER_STEAM_TAB_SERVER_H

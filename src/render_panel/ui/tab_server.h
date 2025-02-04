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

    // client
    class ClientContext;
    class StreamContent;

    class TabServer : public TabBase {
    public:
        explicit TabServer(const std::shared_ptr<GrApplication>& app, QWidget *parent);
        ~TabServer() override;

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        QPixmap qr_pixmap_;

        // client
        std::shared_ptr<ClientContext> client_ctx_ = nullptr;
        StreamContent* stream_content_ = nullptr;

    };
}

#endif //TC_SERVER_STEAM_TAB_SERVER_H

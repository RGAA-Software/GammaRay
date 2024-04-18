//
// Created by hy on 2024/4/9.
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

    class TabServer : public TabBase {
    public:
        explicit TabServer(const std::shared_ptr<GrContext>& ctx, QWidget *parent);
        ~TabServer() override;

        void OnTabShow() override;
        void OnTabHide() override;

    private:

        QString GetItemIconStyleSheet(const QString& url);
        void RefreshVigemState(bool ok);

    private:

        QPixmap qr_pixmap_;
        QStackedWidget* rn_stack_ = nullptr;
        RnApp* rn_app_ = nullptr;
        RnEmpty* rn_empty_ = nullptr;
        QLabel* vigem_state_ = nullptr;

        std::shared_ptr<MessageListener> msg_listener_ = nullptr;

    };
}

#endif //TC_SERVER_STEAM_TAB_SERVER_H

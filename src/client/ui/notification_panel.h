//
// Created by RGAA on 6/07/2024.
//

#ifndef GAMMARAYPC_NOTIFICATION_PANEL_H
#define GAMMARAYPC_NOTIFICATION_PANEL_H

#include "base_widget.h"

#include <QListWidget>
#include <unordered_map>
#include <memory>
#include <map>
#include "transfer/file_transfer_events.h"

namespace tc
{

    class ClientContext;
    class MessageListener;
    class NotificationItem;

    class NotificationPanel : public BaseWidget {
    public:
        explicit NotificationPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;

    private:
        void AddItem(const EvtFileTransferReady& evt);
        void ClearCompletedNotifications();

    private:
        QListWidget* list_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::unordered_map<std::string, NotificationItem*> notifications_;
    };

}

#endif //GAMMARAYPC_NOTIFICATION_PANEL_H

//
// Created by RGAA on 2023/8/14.
//

#ifndef SAILFISH_CLIENT_PC_APPSTREAMLIST_H
#define SAILFISH_CLIENT_PC_APPSTREAMLIST_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QString>
#include <QPaintEvent>

#include "db/stream_item.h"
#include "round_rect_widget.h"

namespace tc
{

    class ClientContext;
    class StreamDBManager;
    class Application;
    class StreamContent;
    class MessageListener;

    using OnItemDoubleClickedCallback = std::function<void(const StreamItem&)>;

    class AppStreamList : public RoundRectWidget {
    public:

        explicit AppStreamList(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        ~AppStreamList() override;

        void paintEvent(QPaintEvent *event) override;

        void LoadStreamItems();

    private:
        QListWidgetItem* AddItem(const StreamItem& item);
        void RegisterActions(int index);
        void ProcessAction(int index, const StreamItem& item);

        void CreateLayout();
        void Init();

        void DeleteStream(const StreamItem& item);
        void StartStream(const StreamItem& item);
        void StopStream(const StreamItem& item);
        void EditStream(const StreamItem& item);

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<StreamDBManager> db_mgr_ = nullptr;
        std::vector<StreamItem> streams_;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        QListWidget* stream_list_ = nullptr;

        Application* application_ = nullptr;
        StreamContent* stream_content_ = nullptr;
    };

}

#endif //SAILFISH_CLIENT_PC_APPSTREAMLIST_H

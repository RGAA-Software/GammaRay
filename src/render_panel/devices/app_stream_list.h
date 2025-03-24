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
#include <map>
#include <QProcess>

#include "client/db/stream_item.h"

namespace tc
{

    class GrContext;
    class GrSettings;
    class StreamDBManager;
    class StreamContent;
    class MessageListener;

    using OnItemDoubleClickedCallback = std::function<void(const StreamItem&)>;

    class AppStreamList : public QWidget {
    public:
        explicit AppStreamList(const std::shared_ptr<GrContext>& ctx, QWidget* parent = nullptr);
        ~AppStreamList() override;

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
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<StreamDBManager> db_mgr_ = nullptr;
        std::vector<StreamItem> streams_;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        QListWidget* stream_list_ = nullptr;
        std::map<std::string, std::shared_ptr<QProcess>> running_processes_;
        StreamContent* stream_content_ = nullptr;
    };

}

#endif //SAILFISH_CLIENT_PC_APPSTREAMLIST_H

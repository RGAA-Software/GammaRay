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

#include "tc_spvr_client/spvr_stream.h"

namespace tc
{

    class GrContext;
    class GrSettings;
    class StreamDBOperator;
    class StreamContent;
    class MessageListener;
    class RunningStreamManager;
    class StreamStateChecker;

    using OnItemDoubleClickedCallback = std::function<void(const std::shared_ptr<spvr::SpvrStream>&)>;

    class AppStreamList : public QWidget {
    public:
        explicit AppStreamList(const std::shared_ptr<GrContext>& ctx, QWidget* parent = nullptr);
        ~AppStreamList() override;

        void LoadStreamItems();

    private:
        QListWidgetItem* AddItem(const std::shared_ptr<spvr::SpvrStream>& item, int index);
        QWidget* GetItemByStreamId(const std::string& stream_id);
        void RegisterActions(int index);
        void ProcessAction(int index, const std::shared_ptr<spvr::SpvrStream>& item);

        void CreateLayout();
        void Init();

        void DeleteStream(const std::shared_ptr<spvr::SpvrStream>& item);
        void StartStream(const std::shared_ptr<spvr::SpvrStream>& item, bool force_only_viewing);
        void StopStream(const std::shared_ptr<spvr::SpvrStream>& item);
        void LockDevice(const std::shared_ptr<spvr::SpvrStream>& item);
        void RestartDevice(const std::shared_ptr<spvr::SpvrStream>& item);
        void ShutdownDevice(const std::shared_ptr<spvr::SpvrStream>& item);
        void EditStream(const std::shared_ptr<spvr::SpvrStream>& item);
        void ShowSettings(const std::shared_ptr<spvr::SpvrStream>& item);

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<StreamDBOperator> db_mgr_ = nullptr;
        std::vector<std::shared_ptr<spvr::SpvrStream>> streams_;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        QListWidget* stream_list_ = nullptr;
        StreamContent* stream_content_ = nullptr;
        std::shared_ptr<RunningStreamManager> running_stream_mgr_ = nullptr;
        // online state checker
        std::shared_ptr<StreamStateChecker> state_checker_ = nullptr;

    };

}

#endif //SAILFISH_CLIENT_PC_APPSTREAMLIST_H

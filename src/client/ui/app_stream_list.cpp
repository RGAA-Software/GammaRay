//
// Created by RGAA on 2023/8/14.
//

#include "app_stream_list.h"

#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QtWidgets/QMenu>
#include <QWidget>
#include <QProcess>

#include "db/stream_item.h"
#include "db/stream_db_manager.h"
#include "client/ct_client_context.h"
#include "tc_common_new/log.h"
#include "widget_helper.h"
#include "message_dialog.h"
#include "stream_item_widget.h"
#include "client/ct_application.h"
#include "client/ct_app_message.h"
#include "create_stream_dialog.h"
#include "stream_content.h"
#include "client/ct_settings.h"
#include "tc_qt_widget/sized_msg_box.h"

namespace tc
{

    class MainItemDelegate : public QStyledItemDelegate {
    public:
        explicit MainItemDelegate(QObject* pParent) {}
        ~MainItemDelegate() override = default;

        void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
            editor->setGeometry(option.rect);
        }
    };

    // - - -- - - -- - - - -- -

    AppStreamList::AppStreamList(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : QWidget(parent) {
        context_ = ctx;
        settings_ = Settings::Instance();
        db_mgr_ = context_->GetDBManager();
        stream_content_ = (StreamContent*)parent;
        application_ = (Application*)parent->parent();

        CreateLayout();
        Init();

        setStyleSheet("background-color: #ffffff;");
    }

    AppStreamList::~AppStreamList() = default;

    void AppStreamList::CreateLayout() {
        auto root_layout = new QHBoxLayout();
        WidgetHelper::ClearMargin(root_layout);

        auto delegate = new MainItemDelegate(this);
        stream_list_ = new QListWidget(this);
        stream_list_->setItemDelegate(delegate);

        stream_list_->setMovement(QListView::Static);
        stream_list_->setViewMode(QListView::IconMode);
        stream_list_->setFlow(QListView::LeftToRight);
        stream_list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        stream_list_->setResizeMode(QListWidget::Adjust);
        stream_list_->setContextMenuPolicy(Qt::CustomContextMenu);
        stream_list_->setSpacing(10);
        stream_list_->setStyleSheet(R"(
            QListWidget::item {
                color: #000000;
                border: transparent;
                border-bottom: 0px solid #dbdbdb;
            }

            QListWidget::item:hover {
                background-color: #DEF0FE;
            }

            QListWidget::item:selected {
                border-left: 0px solid #777777;
            }
        )");

        QObject::connect(stream_list_, &QListWidget::customContextMenuRequested, this, [=](const QPoint& pos) {
            QListWidgetItem* cur_item = stream_list_->itemAt(pos);
            if (cur_item == nullptr) { return; }
            int index = stream_list_->row(cur_item);
            RegisterActions(index);
        });

        QObject::connect(stream_list_, &QListWidget::itemDoubleClicked, this, [=, this](QListWidgetItem *item) {
            int index = stream_list_->row(item);
            StreamItem stream_item = streams_.at(index);
            StartStream(stream_item);
        });

        root_layout->addSpacing(10);
        root_layout->addWidget(stream_list_);
        root_layout->addSpacing(10);

        setLayout(root_layout);
    }

    void AppStreamList::Init() {
        msg_listener_ = context_->ObtainMessageListener();
        msg_listener_->Listen<StreamItemAdded>([=, this](const StreamItemAdded& msg) {
            auto item = msg.item_;
            db_mgr_->AddStream(item);
            LoadStreamItems();
        });

        msg_listener_->Listen<StreamItemUpdated>([=, this](const StreamItemUpdated& msg) {
            db_mgr_->UpdateStream(msg.item_);
            LoadStreamItems();
            LOGI("Update stream : {}", msg.item_.stream_id);
        });
    }

    void AppStreamList::RegisterActions(int index) {
        std::vector<QString> actions = {
            tr("Start"),
            tr("Stop"),
            "",
            tr("Edit"),
            tr("Delete"),
        };
        QMenu* menu = new QMenu();
        for (int i = 0; i < actions.size(); i++) {
            QString action_name = actions.at(i);
            if (action_name.isEmpty()) {
                menu->addSeparator();
                continue;
            }

            QAction* action = new QAction(action_name, menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, this, [=, this]() {
                ProcessAction(i, streams_.at(index));
            });
        }
        menu->exec(QCursor::pos());
        delete menu;
    }

    void AppStreamList::ProcessAction(int index, const StreamItem& item) {
        if (index == 0) {
            // start
            StartStream(item);
        }
        else if (index == 1) {
            // stop
            StopStream(item);
        }
        else if (index == 3) {
            // edit
            EditStream(item);
        }
        else if (index == 4) {
            // delete
            DeleteStream(item);
        }
    }

    void AppStreamList::StartStream(const StreamItem& item) {
        //context_->SendAppMessage(item);
        auto process = std::make_shared<QProcess>();
        QStringList arguments;
        arguments << std::format("--host={}", item.stream_host).c_str()
                  << std::format("--port={}", item.stream_port).c_str()
                  << std::format("--audio={}", settings_->IsAudioEnabled() ? 1 : 0).c_str()
                  << std::format("--clipboard={}", settings_->IsClipboardEnabled() ? 1 : 0).c_str()
                  << std::format("--device_id={}", settings_->device_id_).c_str()
                  << std::format("--stream_id={}", item.stream_id).c_str()
                  << std::format("--network_type={}", item.network_type_).c_str()
                  ;
        LOGI("Start client inner args:");
        for (auto& arg : arguments) {
            LOGI("{}", arg.toStdString());
        }
        process->start("./GammaRayClientInner.exe", arguments);
        running_processes_.erase(item.stream_id);
        running_processes_.insert({item.stream_id, process});
    }

    void AppStreamList::StopStream(const StreamItem& item) {
        context_->SendAppMessage(ClearWorkspace {
            .item_ = item,
        });

        if (running_processes_.contains(item.stream_id)) {
            auto process = running_processes_[item.stream_id];
            if (process) {
                auto msg_box = SizedMessageBox::MakeOkCancelBox(tr("Stop Stream"), tr("Do you want to stop the stream ?"));
                if (msg_box->exec() == 0) {
                    process->kill();
                    running_processes_.erase(item.stream_id);
                }
            }
        }
    }

    void AppStreamList::EditStream(const StreamItem& item) {
        CreateStreamDialog dialog(context_, item);
        dialog.exec();
    }

    void AppStreamList::DeleteStream(const StreamItem& item) {
        auto alert = MessageDialog::Make(context_, tr("Do you want to *DELETE* the stream ?"));
        if (alert->exec() == DialogButton::kCancel) {
            return;
        }

        auto mgr = context_->GetDBManager();
        mgr->DeleteStream(item._id);

        LoadStreamItems();
    }

    QListWidgetItem* AppStreamList::AddItem(const StreamItem& stream) {
        auto item = new QListWidgetItem(stream_list_);
        item->setSizeHint(QSize(230, 128 + 15));
        auto widget = new StreamItemWidget(stream, stream.bg_color, stream_list_);

        auto root_layout = new QVBoxLayout();
        WidgetHelper::ClearMargin(root_layout);
        root_layout->setContentsMargins(2, 0, 2, 0);

        auto layout = new QVBoxLayout();
        layout->addStretch();
        WidgetHelper::ClearMargin(layout);
        root_layout->addLayout(layout);

        auto gap = 0;//5;

        // name
        auto name = new QLabel(stream_list_);
        name->hide();
        name->setObjectName("st_name");
        name->setText(stream.stream_name.c_str());
        name->setStyleSheet(R"(color:#386487; font-size:14px; font-weight:bold; background-color:#909099;)");
        layout->addWidget(name);

        // host
        auto host = new QLabel(stream_list_);
        host->hide();
        host->setObjectName("st_host");
        host->setText(stream.stream_host.c_str());
        host->setStyleSheet(R"(color:#386487; font-size:14px; )");
        layout->addSpacing(gap);
        layout->addWidget(host);

        //
        auto port = new QLabel(stream_list_);
        port->hide();
        port->setObjectName("st_port");
        port->setText(std::to_string(stream.stream_port).c_str());
        port->setStyleSheet(R"(color:#386487; font-size:14px; )");
        layout->addSpacing(gap);
        layout->addWidget(port);

        //
        auto bitrate = new QLabel(stream_list_);
        bitrate->hide();
        bitrate->setObjectName("st_bitrate");
        std::string bt_str = std::to_string(stream.encode_bps) + " Mbps";
        bitrate->setText(bt_str.c_str());
        bitrate->setStyleSheet(R"(color:#386487; font-size:14px; )");
        layout->addSpacing(gap);
        layout->addWidget(bitrate);

        auto fps = new QLabel(stream_list_);
        fps->hide();
        fps->setObjectName("st_fps");
        std::string fps_str = std::to_string(stream.encode_fps) + " FPS";
        fps->setText(fps_str.c_str());
        fps->setStyleSheet(R"(color:#386487; font-size:14px; )");
        layout->addSpacing(gap);
        layout->addWidget(fps);

        //layout->addStretch();

        root_layout->addLayout(layout);
        //layout->addSpacing(6);
        widget->setLayout(root_layout);
        stream_list_->setItemWidget(item, widget);
        return item;
    }

    void AppStreamList::LoadStreamItems() {
        auto db_mgr = context_->GetDBManager();
        streams_ = db_mgr->GetAllStreams();

        context_->PostUITask([=, this]() {
            int count = stream_list_->count();
            for (int i = 0; i < count; i++) {
                auto item = stream_list_->takeItem(0);
                delete item;
            }
            for (const auto& stream : streams_) {
                AddItem(stream);
            }

            if (!streams_.empty()) {
                stream_content_->HideEmptyTip();
            }
            else {
                stream_content_->ShowEmptyTip();
            }
        });
    }

}
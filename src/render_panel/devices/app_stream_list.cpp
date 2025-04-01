//
// Created by RGAA on 2023/8/14.
//

#include "app_stream_list.h"

#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QtWidgets/QMenu>
#include <QWidget>
#include <QProcess>

#include "stream_db_manager.h"
#include "tc_common_new/log.h"
#include "widget_helper.h"
#include "stream_item_widget.h"
#include "create_stream_dialog.h"
#include "stream_content.h"
#include "tc_common_new/base64.h"
#include "tc_dialog.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_app_messages.h"
#include "running_stream_manager.h"
#include "tc_common_new/uid_spacer.h"
#include "edit_relay_stream_dialog.h"
#include "stream_settings_dialog.h"

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

    AppStreamList::AppStreamList(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : QWidget(parent) {
        context_ = ctx;
        settings_ = GrSettings::Instance();
        db_mgr_ = context_->GetStreamDBManager();
        stream_content_ = (StreamContent*)parent;
        running_stream_mgr_ = context_->GetRunningStreamManager();
        CreateLayout();
        Init();

        setStyleSheet("background-color: #ffffff;");
    }

    AppStreamList::~AppStreamList() = default;

    void AppStreamList::CreateLayout() {
        auto root_layout = new QHBoxLayout();
        WidgetHelper::ClearMargins(root_layout);

        auto delegate = new MainItemDelegate(this);
        stream_list_ = new QListWidget(this);
        stream_list_->setItemDelegate(delegate);

        stream_list_->setMovement(QListView::Static);
        stream_list_->setViewMode(QListView::IconMode);
        stream_list_->setFlow(QListView::LeftToRight);
        stream_list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        stream_list_->setResizeMode(QListWidget::Adjust);
        stream_list_->setContextMenuPolicy(Qt::CustomContextMenu);
        stream_list_->setSpacing(15);
        stream_list_->setStyleSheet(R"(
            QListWidget::item {
                color: #000000;
                border: transparent;
                border-bottom: 0px solid #dbdbdb;
            }

            QListWidget::item:hover {
                background-color: none;
            }

            QListWidget::item:selected {
                border-left: 0px solid #777777;
                background-color: none;
            }
        )");

        QObject::connect(stream_list_, &QListWidget::customContextMenuRequested, this, [=, this](const QPoint& pos) {
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
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<StreamItemAdded>([=, this](const StreamItemAdded& msg) {
            auto item = msg.item_;
            if (!db_mgr_->HasStream(item.stream_id_)) {
                db_mgr_->AddStream(item);
            }
            else {
                //TODO: update...

            }
            LoadStreamItems();
        });

        msg_listener_->Listen<StreamItemUpdated>([=, this](const StreamItemUpdated& msg) {
            db_mgr_->UpdateStream(msg.item_);
            LoadStreamItems();
            LOGI("Update stream : {}", msg.item_.stream_id_);
        });
    }

    void AppStreamList::RegisterActions(int index) {
        std::vector<QString> actions = {
            tr("Start"),
            tr("Stop"),
            "",
            tr("Edit"),
            tr("Delete"),
            "",
            tr("Settings"),
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
        // "" 2
        else if (index == 3) {
            // edit
            EditStream(item);
        }
        else if (index == 4) {
            // delete
            DeleteStream(item);
        }
        // "" 5
        else if (index == 6) {
            ShowSettings(item);
        }
    }

    void AppStreamList::StartStream(const StreamItem& item) {
        running_stream_mgr_->StartStream(item);
    }

    void AppStreamList::StopStream(const StreamItem& item) {
        running_stream_mgr_->StopStream(item);
    }

    void AppStreamList::EditStream(const StreamItem& item) {
        if (item.IsRelay()) {
            auto dialog = new EditRelayStreamDialog(context_, item);
            dialog->show();
        }
        else {
            auto dialog = new CreateStreamDialog(context_, item);
            dialog->show();
        }
    }

    void AppStreamList::DeleteStream(const StreamItem& item) {
        auto dlg = TcDialog::Make(tr("Warning"), tr("Do you want to delete the remote control?"), nullptr);
        dlg->SetOnDialogSureClicked([=, this]() {
            auto mgr = context_->GetStreamDBManager();
            mgr->DeleteStream(item._id);
            LoadStreamItems();
        });
        dlg->show();
    }

    void AppStreamList::ShowSettings(const StreamItem& item) {
        auto dialog = new StreamSettingsDialog(context_, item);
        dialog->show();
    }

    QListWidgetItem* AppStreamList::AddItem(const StreamItem& stream) {
        auto item = new QListWidgetItem(stream_list_);
        item->setSizeHint(QSize(230, 150));
        auto widget = new StreamItemWidget(stream, stream.bg_color_, stream_list_);
        WidgetHelper::AddShadow(widget, 0xbbbbbb, 8);

        auto root_layout = new QVBoxLayout();
        WidgetHelper::ClearMargins(root_layout);
        root_layout->setContentsMargins(2, 0, 2, 0);

        auto layout = new QVBoxLayout();
        layout->addStretch();
        WidgetHelper::ClearMargins(layout);
        root_layout->addLayout(layout);

        auto gap = 0;//5;

        // name
        auto name = new QLabel(stream_list_);
        name->hide();
        name->setObjectName("st_name");
        auto stream_name = stream.stream_name_;
        if (stream.IsRelay()) {
            stream_name = tc::SpaceId(stream_name);
        }
        name->setText(stream_name.c_str());
        name->setStyleSheet(R"(color:#386487; font-size:14px; font-weight:bold; background-color:#909099;)");
        layout->addWidget(name);

        // host
        auto host = new QLabel(stream_list_);
        host->hide();
        host->setObjectName("st_host");
        host->setText(stream.stream_host_.c_str());
        host->setStyleSheet(R"(color:#386487; font-size:14px; )");
        layout->addSpacing(gap);
        layout->addWidget(host);

        //
        auto port = new QLabel(stream_list_);
        port->hide();
        port->setObjectName("st_port");
        port->setText(std::to_string(stream.stream_port_).c_str());
        port->setStyleSheet(R"(color:#386487; font-size:14px; )");
        layout->addSpacing(gap);
        layout->addWidget(port);

        //
        auto bitrate = new QLabel(stream_list_);
        bitrate->hide();
        bitrate->setObjectName("st_bitrate");
        std::string bt_str = std::to_string(stream.encode_bps_) + " Mbps";
        bitrate->setText(bt_str.c_str());
        bitrate->setStyleSheet(R"(color:#386487; font-size:14px; )");
        layout->addSpacing(gap);
        layout->addWidget(bitrate);

        auto fps = new QLabel(stream_list_);
        fps->hide();
        fps->setObjectName("st_fps");
        std::string fps_str = std::to_string(stream.encode_fps_) + " FPS";
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
        auto db_mgr = context_->GetStreamDBManager();
        streams_ = db_mgr->GetAllStreamsSortByCreatedTime();

        context_->PostUITask([=, this]() {
            int count = stream_list_->count();
            for (int i = 0; i < count; i++) {
                auto item = stream_list_->takeItem(0);
                delete item;
            }
            for (auto& stream : streams_) {
                stream.device_id_ = settings_->device_id_;
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
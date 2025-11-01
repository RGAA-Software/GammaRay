//
// Created by RGAA on 23/10/2025.
//

#include "st_network_search.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/gr_settings.h"
#include "tc_dialog.h"
#include "tc_label.h"
#include "tc_pushbutton.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/time_util.h"
#include "render_panel/spvr_scanner/spvr_scanner.h"
#include "render_panel/companion/panel_companion.h"
#include <QCheckBox>
#include <QStyledItemDelegate>

namespace tc
{

    // Delegate
    class NtSearchItemDelegate : public QStyledItemDelegate {
    public:
        explicit NtSearchItemDelegate(QObject* pParent) {}
        ~NtSearchItemDelegate() override = default;

        void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
            editor->setGeometry(option.rect);
        }
    };

    // Item
    class NtSearchItem : public QWidget {
    private:
        StNetworkSearch* container_ = nullptr;
        QCheckBox* cb_ = nullptr;

    public:
        NtSearchItem(StNetworkSearch* container, int index, const std::shared_ptr<StNetworkSpvrAccessInfo>& item_info, QWidget* parent) : QWidget(parent) {
            container_ = container;
            auto root_layout = new NoMarginVLayout();
            {
                auto layout = new NoMarginHLayout();
                layout->addSpacing(5);
                // index
                {
                    auto lbl = new TcLabel(this);
                    lbl->setStyleSheet("font-weight: bold;");
                    lbl->setFixedWidth(10);
                    lbl->setText(QString::number(index+1));
                    layout->addWidget(lbl);
                }

                layout->addSpacing(5);
                // selected
                {
                    auto cb = new QCheckBox(this);
                    cb_ = cb;
                    layout->addWidget(cb);
                    connect(cb, &QCheckBox::clicked, this, [=, this]() {
                        container_->OnItemClicked(index, item_info);
                    });
                }

                layout->addSpacing(5);
                // prefix
                {
                    auto lbl = new TcLabel(this);
                    lbl->setStyleSheet("font-weight: bold;");
                    lbl->setFixedWidth(180);
                    auto text = std::format("Spvr: {}:{}", item_info->spvr_ip_, item_info->spvr_port_);
                    lbl->setText(text.c_str());
                    layout->addWidget(lbl);
                }

                layout->addSpacing(5);
                // timestamp
                {
                    auto lbl = new TcLabel(this);
                    lbl->setStyleSheet("font-weight: bold;");
                    lbl->setFixedWidth(145);
                    auto text = std::format("{}", TimeUtil::FormatTimestamp(item_info->update_timestamp_));
                    lbl->setText(text.c_str());
                    layout->addWidget(lbl);
                }

                layout->addSpacing(5);
                // origin
                {
                    auto lbl = new TcLabel(this);
                    lbl->setText(item_info->origin_info_.c_str());
                    layout->addWidget(lbl);
                }

                layout->addSpacing(5);
                root_layout->addLayout(layout);
            }
            setLayout(root_layout);
        }

        void Unselect() {
            cb_->setChecked(false);
        }

    };

    StNetworkSearch::StNetworkSearch(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        app_ = app;
        context_ = app_->GetContext();
        setFixedSize(640, 480);
        CreateLayout();
        CenterDialog(this);

        msg_listener_ = context_->ObtainMessageListener();
        // msg_listener_->Listen<MsgSpvrAccessInfo>([=, this](const MsgSpvrAccessInfo& msg) {
        //     context_->PostUITask([=, this]() {
        //         this->UpdateItems();
        //     });
        // });
        this->UpdateItems();
    }

    StNetworkSearch::~StNetworkSearch() {

    }

    void StNetworkSearch::closeEvent(QCloseEvent *) {
        done(1);
    }

    void StNetworkSearch::CreateLayout() {
        setWindowTitle(tcTr("id_file_trans_search"));
        auto settings = GrSettings::Instance();

        auto item_width = 600;

        auto delegate = new NtSearchItemDelegate(this);
        list_widget_ = new QListWidget(this);
        list_widget_->setItemDelegate(delegate);

        list_widget_->setMovement(QListView::Static);
        list_widget_->setViewMode(QListView::ListMode);
        list_widget_->setFlow(QListView::TopToBottom);
        list_widget_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list_widget_->setResizeMode(QListWidget::Adjust);
        list_widget_->setContextMenuPolicy(Qt::CustomContextMenu);
        list_widget_->setSpacing(2);
        list_widget_->setStyleSheet(R"(
            QListWidget {
                background-color: #ffffff;
                border: 0px solid #ffffff;
            }
            QListWidget::item {
                color: #ffffff;
                border: transparent;
                border-bottom: 0px solid #ffffff;
            }

            QListWidget::item:hover {
                background-color: none;
            }

            QListWidget::item:selected {
                border-left: 0px solid #777777;
                background-color: none;
            }
        )");

        QObject::connect(list_widget_, &QListWidget::customContextMenuRequested, this, [=, this](const QPoint& pos) {
            QListWidgetItem* cur_item = list_widget_->itemAt(pos);
            if (cur_item == nullptr) { return; }
            int index = list_widget_->row(cur_item);

        });

        QObject::connect(list_widget_, &QListWidget::itemDoubleClicked, this, [=, this](QListWidgetItem *item) {
            int index = list_widget_->row(item);

        });

        root_layout_->addWidget(list_widget_);

        // sure button
        {
            auto layout = new NoMarginHLayout();
            auto btn_sure = new TcPushButton();
            btn_sure->SetTextId("id_ok");
            connect(btn_sure, &QPushButton::clicked, this, [=, this] () {
                done(0);
            });

            layout->addStretch();
            layout->addWidget(btn_sure);
            layout->addStretch();
            btn_sure->setFixedSize(QSize(item_width, 35));
            root_layout_->addLayout(layout);
        }

        root_layout_->addSpacing(10);

        // empty
        empty_lbl_ = new TcLabel(this);
        empty_lbl_->SetTextId("id_nothing");
        empty_lbl_->setHidden(true);
    }

    void StNetworkSearch::resizeEvent(QResizeEvent*) {
        auto lbl_size = QSize(150, 45);
        empty_lbl_->setGeometry((this->width() - lbl_size.width())/2,  (this->height() - lbl_size.height())/2, lbl_size.width(), lbl_size.height());
    }

    QListWidgetItem* StNetworkSearch::AddItem(int index, const std::shared_ptr<StNetworkSpvrAccessInfo>& item_info) {
        auto item = new QListWidgetItem(list_widget_);
        auto item_size = QSize(600, 40);
        item->setSizeHint(item_size);
        auto widget = new NtSearchItem(this, index, item_info, list_widget_);
        widget->setFixedSize(item_size);
        list_widget_->setItemWidget(item, widget);
        return item;
    }

    void StNetworkSearch::UpdateItems() {
        int count = list_widget_->count();
        for (int i = 0; i < count; i++) {
            auto item = list_widget_->takeItem(0);
            delete item;
        }

        auto scanner = app_->GetSpvrScanner();
        auto access_info = scanner->GetSpvrAccessInfo();
        int index = 0;
        for (const auto& info : access_info) {
            this->AddItem(index++, info.second);
        }

        empty_lbl_->setHidden(!access_info.empty());
    }

    void StNetworkSearch::OnItemClicked(int index, const std::shared_ptr<StNetworkSpvrAccessInfo>& item_info) {
        selected_item_ = item_info;
        int count = list_widget_->count();
        for (int i = 0; i < count; i++) {
            if (i == index) {
                continue;
            }
            auto item = list_widget_->item(i);
            auto widget = list_widget_->itemWidget(item);
            ((NtSearchItem*)widget)->Unselect();
        }
    }

    std::shared_ptr<StNetworkSpvrAccessInfo> StNetworkSearch::GetSelectedItem() {
        return selected_item_;
    }

}
//
// Created by RGAA on 22/03/2025.
//

#include "tab_profile.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include "no_margin_layout.h"
#include "tc_label.h"
#include "tc_pushbutton.h"
#include "tc_common_new/log.h"
#include "tc_qt_widget/clickable_widget.h"
#include "render_panel/gr_context.h"
#include "tc_account_sdk/acc_sdk.h"
#include "tc_account_sdk/acc_profile.h"
#include "tc_account_sdk/acc_device.h"
#include "tab_profile_device_item.h"
#ifdef WIN32
#include <Windows.h>
#include <shellapi.h>
#endif
#include <QLabel>
#include <QListWidget>
#include <QStyledItemDelegate>

namespace tc
{

    class MyDeviceItemDelegate : public QStyledItemDelegate {
    public:
        explicit MyDeviceItemDelegate(QObject* pParent) {}
        ~MyDeviceItemDelegate() override = default;

        void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
            editor->setGeometry(option.rect);
        }
    };

    TabProfile::TabProfile(const std::shared_ptr<GrApplication>& app, QWidget *parent)
        : TabBase(app, parent) {
#ifdef WIN32
        auto hwnd = HWND(winId());
        ::DragAcceptFiles(hwnd, TRUE);
        ::ChangeWindowMessageFilterEx(hwnd, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
        ::ChangeWindowMessageFilterEx(hwnd, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
        ::ChangeWindowMessageFilterEx(hwnd, 0x0049, MSGFLT_ALLOW, nullptr);
        setAcceptDrops(true);
#endif

        acc_sdk_ = context_->GetAccSdk();
        stacked_widget_ = new QStackedWidget(this);

        root_layout_ = new NoMarginHLayout();
        AddLeftProfileInfo();
        AddRightDetailInfo();
        setLayout(root_layout_);

        //
        QueryMyDevices();
    }

    void TabProfile::OnTabShow() {

    }

    void TabProfile::OnTabHide() {

    }

    void TabProfile::dragEnterEvent(QDragEnterEvent *event) {
        event->accept();
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        }
        LOGI("DragEventEnter....");
    }

    void TabProfile::dragMoveEvent(QDragMoveEvent *event) {
        event->accept();
        LOGI("DragEventMove....");
    }

    void TabProfile::dropEvent(QDropEvent *event) {
        QList<QUrl> urls = event->mimeData()->urls();
        LOGI("DragEventDrop....{}", urls.size());
        if (urls.isEmpty()) {
            return;
        }
        std::vector<QString> files;
        for (const auto& url : urls) {
            files.push_back(url.toLocalFile());
            LOGI("Drop files: {}", url.toLocalFile().toStdString());
        }
        //if (file_transfer_) {
        //    file_transfer_->SendFiles(files);
        //}
    }

    void TabProfile::AddLeftProfileInfo() {
        auto widget = new QWidget(this);
        auto root_layout = new NoMarginVLayout();
        widget->setLayout(root_layout);
        widget->setFixedWidth(360);
        //widget->setStyleSheet("background-color: #eeeeee;");

        root_layout->addSpacing(20);

        // title
        {
            auto lbl = new QLabel(this);
            lbl->setText("Personal Information");
            lbl->setStyleSheet("font-size: 16px; font-weight: 700; color:#555555;");
            root_layout->addWidget(lbl);
        }

        root_layout->addSpacing(20);

        {
            auto item_layout = new NoMarginHLayout();

            // widget
            {
                auto icon = new QLabel(this);
                icon->setFixedSize(64, 64);
                QString style = R"(background-image: url(%1);
                        background-repeat: no-repeat;
                        background-position: center;
                    )";
                icon->setStyleSheet(style.arg(":/resources/image/ic_empty_avatar.svg"));
                item_layout->addWidget(icon);
            }

            // info
            {
                auto account_layout = new NoMarginVLayout();

                // name
                auto lbl_account = new QLabel(this);
                lbl_account->setText("1880000dddd");
                lbl_account->setStyleSheet("font-size: 15px; font-weight: 700; color:#555555;");
                account_layout->addSpacing(10);
                account_layout->addWidget(lbl_account);

                account_layout->addSpacing(10);

                auto lbl_account_type = new QLabel(this);
                lbl_account_type->setFixedSize(85, 20);
                lbl_account_type->setAlignment(Qt::AlignCenter);
                lbl_account_type->setStyleSheet("font-size: 12px; color: #ffffff; background-color: #2979ff;");
                lbl_account_type->setText("Freemium");
                account_layout->addWidget(lbl_account_type);
                account_layout->addSpacing(20);

                item_layout->addSpacing(20);
                item_layout->addLayout(account_layout);
                item_layout->addStretch();
            }

            root_layout->addLayout(item_layout);
        }

        root_layout->addStretch(1000);
        root_layout_->addWidget(widget);
    }

    void TabProfile::AddRightDetailInfo() {
        auto widget = new QWidget(this);
        auto root_layout = new NoMarginVLayout();
        widget->setLayout(root_layout);
        widget->setFixedWidth(800);

        root_layout->addSpacing(20);

        // device details
        {
            auto lbl = new QLabel(this);
            lbl->setText("Device Details");
            lbl->setStyleSheet("font-size: 16px; font-weight: 700; color:#555555; padding-left: 6px;");
            root_layout->addWidget(lbl);
        }

        //
        {
            auto layout = new NoMarginHLayout();
            layout->addSpacing(6);
            auto bg_size = QSize(238, 90);
            auto icon_size = QSize(35, 35);
            // served
            {
                auto bg = new ClickableWidget(this);
                bg->setFixedSize(bg_size);
                bg->SetGradientColor(0xa1c4fd, 0xc2e9fb);
                bg->SetRadius(10);
                layout->addWidget(bg);

                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(12);

                // info
                {
                    auto info_layout = new NoMarginVLayout();
                    info_layout->addSpacing(5);
                    // title
                    auto title = new TcLabel();
                    title->setText("Served Duration");
                    title->setStyleSheet("font-weight: 500; font-size: 14px; color: #ffffff;");
                    info_layout->addSpacing(8);
                    info_layout->addWidget(title);

                    // time
                    auto time = new TcLabel(this);
                    time->setText("1:09:54");
                    time->setStyleSheet("font-weight: 700; font-size: 27px; color: #ffffff;");
                    info_layout->addWidget(time);
                    info_layout->addSpacing(12);
                    info_layout->addStretch();

                    item_layout->addLayout(info_layout);
                }

                // icon
                {
                    auto icon = new QLabel(this);
                    icon->setFixedSize(icon_size);
                    QString style = R"(background-image: url(%1);
                        background-repeat: no-repeat;
                        background-position: center;
                    )";
                    icon->setStyleSheet(style.arg(":/resources/image/ic_served.svg"));
                    item_layout->addSpacing(10);
                    item_layout->addWidget(icon);
                }

                item_layout->addSpacing(12);
                bg->setLayout(item_layout);
            }

            // controlled
            {
                auto bg = new ClickableWidget(this);
                bg->setFixedSize(bg_size);
                bg->SetGradientColor(0xa1c4fd, 0xc2e9fb);
                bg->SetRadius(10);
                layout->addSpacing(30);
                layout->addWidget(bg);

                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(12);

                // info
                {
                    auto info_layout = new NoMarginVLayout();
                    info_layout->addSpacing(5);
                    // title
                    auto title = new TcLabel();
                    title->setText("Controlled Duration");
                    title->setStyleSheet("font-weight: 500; font-size: 14px; color: #ffffff;");
                    info_layout->addSpacing(8);
                    info_layout->addWidget(title);

                    // time
                    auto time = new TcLabel(this);
                    time->setText("2 Hours");
                    time->setStyleSheet("font-weight: 700; font-size: 27px; color: #ffffff;");
                    info_layout->addWidget(time);
                    info_layout->addSpacing(12);
                    info_layout->addStretch();

                    item_layout->addLayout(info_layout);
                }

                // icon
                {
                    auto icon = new QLabel(this);
                    icon->setFixedSize(icon_size);
                    QString style = R"(background-image: url(%1);
                        background-repeat: no-repeat;
                        background-position: center;
                    )";
                    icon->setStyleSheet(style.arg(":/resources/image/ic_controller_hand.svg"));
                    item_layout->addSpacing(10);
                    item_layout->addWidget(icon);
                }

                item_layout->addSpacing(12);
                bg->setLayout(item_layout);
            }

            // device restricts
            {
                auto bg = new ClickableWidget(this);
                bg->setFixedSize(bg_size);
                bg->SetGradientColor(0xa1c4fd, 0xc2e9fb);
                bg->SetRadius(10);
                layout->addSpacing(30);
                layout->addWidget(bg);

                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(12);

                // info
                {
                    auto info_layout = new NoMarginVLayout();
                    info_layout->addSpacing(5);
                    // title
                    auto title = new TcLabel();
                    title->setText("Managed Devices");
                    title->setStyleSheet("font-weight: 500; font-size: 14px; color: #ffffff;");
                    info_layout->addSpacing(8);
                    info_layout->addWidget(title);

                    // time
                    auto time = new TcLabel(this);
                    time->setText("2/100");
                    time->setStyleSheet("font-weight: 700; font-size: 27px; color: #ffffff;");
                    info_layout->addWidget(time);
                    info_layout->addSpacing(12);
                    info_layout->addStretch();

                    item_layout->addLayout(info_layout);
                }

                // icon
                {
                    auto icon = new QLabel(this);
                    icon->setFixedSize(icon_size);
                    QString style = R"(background-image: url(%1);
                        background-repeat: no-repeat;
                        background-position: center;
                    )";
                    icon->setStyleSheet(style.arg(":/resources/image/ic_device_used.svg"));
                    item_layout->addSpacing(10);
                    item_layout->addWidget(icon);
                }

                item_layout->addSpacing(12);
                bg->setLayout(item_layout);

            }
            layout->addStretch();
            root_layout->addSpacing(15);
            root_layout->addLayout(layout);
        }

        root_layout->addSpacing(20);

        // my devices
        {
            auto lbl = new QLabel(this);
            lbl->setText("My Devices");
            lbl->setStyleSheet("font-size: 16px; font-weight: 700; color:#555555; padding-left: 6px;");
            root_layout->addWidget(lbl);
        }

        root_layout->addSpacing(5);

        // my device list
        {
            auto delegate = new MyDeviceItemDelegate(this);
            list_widget_ = new QListWidget(this);
            list_widget_->setFixedWidth(512);
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
                //auto item_info = records_.at(index);

            });

            // list content
            auto content_layout = new NoMarginHLayout();
            content_layout->addWidget(list_widget_);

            // right
            content_layout->addWidget(stacked_widget_);
            stacked_widget_->addWidget(AddEmptyWidget());
            stacked_widget_->addWidget(AddOnlineInfoWidget());
            stacked_widget_->addWidget(AddOfflineInfoWidget());

            stacked_widget_->setCurrentIndex(0);

            root_layout->addLayout(content_layout);
        }

        root_layout_->addWidget(widget);
    }

    QListWidgetItem* TabProfile::AddItem(const std::shared_ptr<AccountDevice>& item_info, int index) {
        auto item = new QListWidgetItem(list_widget_);
        auto item_size = QSize(475, 60);
        item->setSizeHint(item_size);
        auto widget = new TabProfileDeviceItemWidget(app_, item_info, list_widget_);
        widget->setFixedSize(item_size);
        list_widget_->setItemWidget(item, widget);

        widget->SetOnItemClickListener([=, this](QWidget* w, const std::shared_ptr<AccountDevice>& device) {
            if (index == 0) {
                stacked_widget_->setCurrentIndex(0);
            }
            else if (index == 1) {
                stacked_widget_->setCurrentIndex(1);
            }
            else if (index == 2) {
                stacked_widget_->setCurrentIndex(2);
            }
        });

        return item;
    }

    void TabProfile::QueryMyDevices() {
        context_->PostTask([=, this]() {
            auto devices = acc_sdk_->QueryDevices("", 0, 0);
            context_->PostUITask([=, this]() {
                int count = list_widget_->count();
                for (int i = 0; i < count; i++) {
                    auto item = list_widget_->takeItem(0);
                    delete item;
                }

                int index = 0;
                for (const auto& item_info : devices) {
                    AddItem(item_info, index++);
                }
            });
        });
    }

    QWidget* TabProfile::AddEmptyWidget() {
        auto w = new QWidget(this);
        w->setStyleSheet("background-color:#ffffff;");
        return w;
    }

    QWidget* TabProfile::AddOnlineInfoWidget() {
        auto w = new QWidget(this);
        w->setStyleSheet("background-color:#eeeeee;");
        auto root_layout = new NoMarginVLayout();

        return w;
    }

    QWidget* TabProfile::AddOfflineInfoWidget() {
        auto w = new QWidget(this);
        w->setStyleSheet("background-color:#cccccc;");
        return w;
    }

}
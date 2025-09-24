//
// Created by RGAA on 24/09/2025.
//

#include "hw_cpu_detail_widget.h"
#include "no_margin_layout.h"
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QScrollBar>

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

    HWCpuDetailWidget::HWCpuDetailWidget(QWidget* parent) : QWidget(parent) {
        auto root_layout = new NoMarginVLayout();
        auto delegate = new MainItemDelegate(this);
        apps_list_ = new QListWidget(this);
        apps_list_->setItemDelegate(delegate);

        apps_list_->setMovement(QListView::Static);
        apps_list_->setViewMode(QListView::IconMode);
        apps_list_->setFlow(QListView::LeftToRight);
        apps_list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        //apps_list_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        apps_list_->setResizeMode(QListWidget::Adjust);
        apps_list_->setContextMenuPolicy(Qt::CustomContextMenu);
        apps_list_->setSpacing(5);
        apps_list_->verticalScrollBar()->setStyleSheet(R"(
                QScrollBar {
                    width: 10px;
                    border: none;
                }

                QScrollBar::handle:vertical:normal {
                    background:#dddddd;
                    width:10px;
                    border-radius: 5px;
                }
                QScrollBar::handle:vertical:hover {
                    background:#8f8f8f;
                    width:10px;
                    border-radius: 5px;
                }
                QScrollBar::handle:vertical:pressed {
                    background:#8f8f8f;
                    width:10px;
                    border-radius: 5px;
                }
                QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                    border: none;
                    background: none;
                    height: 0px;
                }
                QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
                    background: none;
                }
            )");
        //                    background-color: #f5f5f5;
        //                    border-radius: 10px;
        apps_list_->setStyleSheet(R"(
                QListWidget {
                    background-color: #fafafa;
                    border-radius: 10px;
                    margin: 0px;
                    padding: 0px;
                    outline: 0;
                }
                QListWidget::item {
                    color: #000000;
                    margin: 0px;
                    padding: 0px;
                }

                QListWidget::item:hover {
                    background-color: none;
                }

                QListWidget::item:selected {
                    border-left: 0px solid #777777;
                    background-color: none;
                }

                QListWidget::vertical-scrollbar {
                    border: none;
                }

                QListWidget::horizontal-scrollbar {
                    border: none;
                }
            )");

        connect(apps_list_, &QListWidget::customContextMenuRequested, this, [=, this](const QPoint& pos) {
            QListWidgetItem* cur_item = apps_list_->itemAt(pos);
            if (cur_item == nullptr) { return; }
            int index = apps_list_->row(cur_item);

        });

        connect(apps_list_, &QListWidget::itemDoubleClicked, this, [=, this](QListWidgetItem *item) {
            int index = apps_list_->row(item);
            auto stream_item = cpus_hist_.at(index);
        });

        root_layout->addWidget(apps_list_);
        root_layout->addSpacing(10);
        setLayout(root_layout);
    }

    void HWCpuDetailWidget::UpdateCpusInfo(const std::vector<SysSingleCpuInfo>& cpus) {
        if (!init_) {
            int index = 0;
            for (const auto &c: cpus) {
                this->AddItem(c, index++);
            }
            init_ = true;
        }
        else {
            int count = apps_list_->count();
            for (int i = 0; i < count; i++) {
                auto item = apps_list_->item(i);
                auto widget = (HWCpuDetailItem*)apps_list_->itemWidget(item);
                auto obj_name = widget->objectName();
                for (const auto& cpu : cpus) {
                    if (QString::fromStdString(cpu.name_) == obj_name) {
                        widget->UpdateValue(cpu.usage_/100.0f);
                        break;
                    }
                }
            }
        }
    }

    QListWidgetItem* HWCpuDetailWidget::AddItem(const SysSingleCpuInfo& info, int index) {
        auto item = new QListWidgetItem(apps_list_);
        item->setSizeHint(QSize(92, 50));
        auto widget = new HWCpuDetailItem(info.name_.c_str(), apps_list_);
        widget->setObjectName(info.name_);
        apps_list_->setItemWidget(item, widget);
        return item;
    }

}
//
// Created by RGAA on 28/05/2025.
//

#include "st_security_visitor.h"
#include "no_margin_layout.h"
#include "tc_pushbutton.h"
#include "tc_label.h"
#include "tc_qt_widget/pagination/page_widget.h"
#include "st_security_visitor_item.h"
#include "render_panel/gr_context.h"
#include "render_panel/database/gr_database.h"
#include "render_panel/database/visit_record.h"
#include "render_panel/database/visit_record_operator.h"
#include "tc_dialog.h"
#include "tc_image_button.h"
#include <QStyledItemDelegate>
#include <QMenu>
#include <QClipboard>
#include <QApplication>

namespace tc
{

    constexpr int kPageSize = 20;

    class StSecurityVisitorItemDelegate : public QStyledItemDelegate {
    public:
        explicit StSecurityVisitorItemDelegate(QObject* pParent) {}
        ~StSecurityVisitorItemDelegate() override = default;

        void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
            editor->setGeometry(option.rect);
        }
    };

    StSecurityVisitor::StSecurityVisitor(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginVLayout();

        {
            // title
            auto layout = new NoMarginHLayout();

            auto label = new TcLabel(this);
            label->setFixedWidth(235);
            label->SetTextId("id_security_visitor_history_logs");
            label->setStyleSheet("font-size: 16px; font-weight: 700; padding-left: 7px;");
            layout->addWidget(label);

            {
                auto btn_refresh = new TcImageButton(":/resources/image/ic_refresh.svg", QSize(16, 16));
                btn_refresh->SetColor(0xffffff, 0xdddddd, 0xbbbbbb);
                btn_refresh->SetRoundRadius(13);
                btn_refresh->setFixedSize(26, 26);
                layout->addWidget(btn_refresh, 0, Qt::AlignVCenter);
                btn_refresh->SetOnImageButtonClicked([=, this]() {
                    LoadPage(page_widget_->getCurrentPage());
                });
            }

            {
                auto btn_clear_all = new TcImageButton(":/resources/image/ic_clear.svg", QSize(16, 16));
                btn_clear_all->SetColor(0xffffff, 0xdddddd, 0xbbbbbb);
                btn_clear_all->SetRoundRadius(13);
                btn_clear_all->setFixedSize(26, 26);
                layout->addSpacing(10);
                layout->addWidget(btn_clear_all, 0, Qt::AlignVCenter);
                btn_clear_all->SetOnImageButtonClicked([=, this]() {
                    TcDialog dialog("Warning", "Do you want to delete ALL records?");
                    if (dialog.exec() == kDoneOk) {
                        context_->PostTask([=, this]() {
                            visit_op_->DeleteAll();
                            LoadPage(page_widget_->getCurrentPage());
                        });
                    }
                });
            }

            layout->addStretch();
            root_layout->addLayout(layout);
        }

        {
            auto delegate = new StSecurityVisitorItemDelegate(this);
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
                RegisterActions(index);
            });

            QObject::connect(list_widget_, &QListWidget::itemDoubleClicked, this, [=, this](QListWidgetItem *item) {
                int index = list_widget_->row(item);
                auto record = records_.at(index);
                ProcessCopy(record);
            });

            root_layout->addWidget(list_widget_);

        }

        page_widget_ = new PageWidget();
        page_widget_->setMaxPage(1);

        root_layout->addSpacing(10);
        root_layout->addWidget(page_widget_);
        root_layout->addSpacing(20);

        setLayout(root_layout);

        setObjectName("StSecurityVisitor");
        setStyleSheet("#StSecurityVisitor {background-color: #ffffff;}");

        page_widget_->setSelectedCallback([=, this](int page) {
            LOGI("Will load page: {}", page);
            LoadPage(page);
        });

        // test beg //
        //context_->PostDBTask([=, this]() {
        //    for (int i = 0; i < 26; i++) {
        //        auto record = std::make_shared<VisitRecord>(VisitRecord {
        //            .conn_type_ = "TT",
        //            .begin_ = 1 + i,
        //            .end_ = 1 * i,
        //            .duration_ = 10,
        //            .account_ = std::format("acc: {}", i),
        //            .controller_device_ = "xxxx",
        //            .controlled_device_ = "aaaa",
        //        });
        //        visit_op_->InsertVisitRecord(record);
        //    }
        //});
        // test end //

        // Load Page 1
        context_->PostUIDelayTask([=, this]() {
            LoadPage(1);
        }, 500);
    }

    QListWidgetItem* StSecurityVisitor::AddItem(const std::shared_ptr<VisitRecord>& item_info) {
        auto item = new QListWidgetItem(list_widget_);
        auto item_size = QSize(995, 45);
        item->setSizeHint(item_size);
        auto widget = new StSecurityVisitorItemWidget(app_, item_info, list_widget_);
        widget->setFixedSize(item_size);
        list_widget_->setItemWidget(item, widget);
        return item;
    }

    void StSecurityVisitor::LoadPage(int page) {
        if (!list_widget_) {
            return;
        }

        context_->PostDBTask([=, this]() {
            if (!visit_op_) {
                visit_op_ = context_->GetDatabase()->GetVisitRecordOp();
            }
            records_.clear();
            auto total_count = visit_op_->GetTotalCounts();

            records_.push_back(std::make_shared<VisitRecord>(VisitRecord {
                .conn_type_ = "",
                .begin_ = 1,
                .end_ = 1,
                .duration_ = 10,
                .visitor_device_ = "",
                .target_device_ = "",
            }));

            auto records = visit_op_->QueryVisitRecords(page, kPageSize);
            for (const auto& r : records) {
                records_.push_back(r);
            }

            context_->PostUITask([=, this]() {

                page_widget_->setMaxPage(total_count/kPageSize+1);

                auto index = 0;
                int count = list_widget_->count();
                for (int i = 0; i < count; i++) {
                    auto item = list_widget_->takeItem(0);
                    delete item;
                }

                for (const auto& item_info : records_) {
                    AddItem(item_info);
                }
            });
        });
    }

    void StSecurityVisitor::RegisterActions(int index) {
        auto record = records_.at(index);
        std::vector<QString> actions = {
                tr("Copy"),
                tr("Copy As Json"),
                "",
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
                if (i == 0) {
                    ProcessCopy(record);
                }
                else if (i == 1) {
                    ProcessCopyAsJson(record);
                }
                else if (i == 3) {
                    ProcessDelete(record);
                }
            });
        }
        menu->exec(QCursor::pos());
        delete menu;
    }

    void StSecurityVisitor::ProcessCopy(const std::shared_ptr<VisitRecord>& record) {
        auto msg = record->AsString();
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(QString::fromStdString(msg));
        context_->NotifyAppMessage("Copy Success", "Information has been written to the Clipboard");
    }

    void StSecurityVisitor::ProcessCopyAsJson(const std::shared_ptr<VisitRecord>& record) {
        auto msg = record->AsJson();
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(QString::fromStdString(msg));
        context_->NotifyAppMessage("Copy Success", "Information has been written to the Clipboard");
    }

    void StSecurityVisitor::ProcessDelete(const std::shared_ptr<VisitRecord>& record) {
        TcDialog dialog("Delete", "Do you want to delete this record?");
        if (dialog.exec() == kDoneOk) {
            visit_op_->Delete(record->id_);
            auto current_page = page_widget_->getCurrentPage();
            LoadPage(current_page);
        }
    }

}
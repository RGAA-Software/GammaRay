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
#include "render_panel/database/visit_record.h"
#include <QStyledItemDelegate>

namespace tc
{

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
            auto label = new TcLabel(this);
            label->SetTextId("id_security_visitor_history_logs");
            label->setStyleSheet("font-size: 16px; font-weight: 700; padding-left: 7px;");
            root_layout->addWidget(label);
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

            });

            QObject::connect(list_widget_, &QListWidget::itemDoubleClicked, this, [=, this](QListWidgetItem *item) {
                int index = list_widget_->row(item);
                auto item_info = records_.at(index);

            });

            root_layout->addWidget(list_widget_);

        }

        page_widget_ = new PageWidget();
        page_widget_->setMaxPage(5550);

        root_layout->addSpacing(10);
        root_layout->addWidget(page_widget_);
        root_layout->addSpacing(20);

        setLayout(root_layout);

        setObjectName("StSecurityVisitor");
        setStyleSheet("#StSecurityVisitor {background-color: #ffffff;}");

        // Load Page 1
        LoadPage(1);
    }

    QListWidgetItem* StSecurityVisitor::AddItem(const std::shared_ptr<VisitRecord>& item_info) {
        auto item = new QListWidgetItem(list_widget_);
        auto item_size = QSize(995, 40);
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

        context_->PostTask([=, this]() {
            // TEST
            std::string conn_type_;
            int64_t begin_{0};
            int64_t end_{0};
            int64_t duration_{0};
            std::string account_;
            std::string controller_device_;
            std::string controlled_device_;
            records_.push_back(std::make_shared<VisitRecord>(VisitRecord {
                .conn_type_ = "",
                .begin_ = 1,
                .end_ = 1,
                .duration_ = 10,
                .account_ = "",
                .controller_device_ = "",
                .controlled_device_ = "",
            }));
            for (int i = 0; i < 20; i++) {
                records_.push_back(std::make_shared<VisitRecord>(VisitRecord {
                    .conn_type_ = "TT",
                    .begin_ = 1,
                    .end_ = 1,
                    .duration_ = 10,
                    .account_ = "",
                    .controller_device_ = "xxxx",
                    .controlled_device_ = "aaaa",
                }));
            }

            context_->PostUITask([=, this]() {
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

}
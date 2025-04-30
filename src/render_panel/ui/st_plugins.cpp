//
// Created by RGAA on 29/04/2025.
//

#include <QStyledItemDelegate>
#include "st_plugins.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "tc_common_new/message_notifier.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"
#include "st_plugin_item_widget.h"
#include "no_margin_layout.h"

namespace tc
{

    class PluginInfoItemDelegate : public QStyledItemDelegate {
    public:
        explicit PluginInfoItemDelegate(QObject* pParent) {}
        ~PluginInfoItemDelegate() override = default;

        void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
            editor->setGeometry(option.rect);
        }
    };

    StPlugins::StPlugins(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginVLayout();
        auto delegate = new PluginInfoItemDelegate(this);
        stream_list_ = new QListWidget(this);
        stream_list_->setItemDelegate(delegate);

        stream_list_->setMovement(QListView::Static);
        stream_list_->setViewMode(QListView::ListMode);
        stream_list_->setFlow(QListView::TopToBottom);
        stream_list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        stream_list_->setResizeMode(QListWidget::Adjust);
        stream_list_->setContextMenuPolicy(Qt::CustomContextMenu);
        stream_list_->setSpacing(0);
        stream_list_->setStyleSheet(R"(
            QListWidget {
                background-color: #ffffff;
            }
            QListWidget::item {
                color: #ffffff;
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

        });

        QObject::connect(stream_list_, &QListWidget::itemDoubleClicked, this, [=, this](QListWidgetItem *item) {
            int index = stream_list_->row(item);
            auto item_info = items_info_.at(index);

        });

        root_layout->addWidget(stream_list_);

        setLayout(root_layout);

        msg_listener_->Listen<MsgPluginsInfo>([=, this](const MsgPluginsInfo& m_info) {
            auto plugins_info = m_info.plugins_info_->plugins_info();
            bool has_new_plugin = false;
            for (int i = 0; i < plugins_info.size(); i++) {
                auto info = plugins_info.at(i);

                bool found = false;
                bool plugin_enabled = true;
                for (const auto& item_info : items_info_) {
                    if (item_info->id_ == info.id()) {
                        found = true;
                        plugin_enabled = info.enabled();
                        break;
                    }
                }

                if (!found) {
                    has_new_plugin = true;
                    auto plugin_info = std::make_shared<PtPluginInfo>();
                    plugin_info->CopyFrom(info);
                    items_info_.push_back(std::make_shared<PluginItemInfo>(PluginItemInfo {
                        .id_ = info.id(),
                        .info_ = plugin_info,
                    }));
                }
                else {
                    for (const auto& item_info : items_info_) {
                        item_info->info_->set_enabled(plugin_enabled);
                    }
                }
            }

            if (has_new_plugin) {
                RefreshListWidget();
            }
            else {
                UpdateItemStatus();
            }
        });

        setObjectName("StPlugins");
        setStyleSheet("#StPlugins {background-color: #ffffff;}");
    }

    void StPlugins::OnTabShow() {

    }

    void StPlugins::OnTabHide() {

    }

    QListWidgetItem* StPlugins::AddItem(const std::shared_ptr<PluginItemInfo>& item_info, int index) {
        auto item = new QListWidgetItem(stream_list_);
        auto item_size = QSize(955, 60);
        item->setSizeHint(item_size);
        auto widget = new StPluginItemWidget(app_, item_info, stream_list_);
        widget->setFixedSize(item_size);
        stream_list_->setItemWidget(item, widget);
        return item;
    }

    void StPlugins::RefreshListWidget() {
        if (!stream_list_) {
            return;
        }
        context_->PostUITask([=, this]() {
            auto index = 0;
            int count = stream_list_->count();
            for (int i = 0; i < count; i++) {
                auto item = stream_list_->takeItem(0);
                delete item;
            }

            for (const auto& item_info : items_info_) {
                AddItem(item_info, index++);
            }
        });
    }

    void StPlugins::UpdateItemStatus() {
        if (stream_list_->count() != items_info_.size()) {
            LOGE("Invalid plugins count: {} => {}", stream_list_->count(), items_info_.size());
            return;
        }
        context_->PostUITask([=, this]() {
            int count = stream_list_->count();
            for (int i = 0; i < count; i++) {
                //LOGI("plugin: {}, name: {}, enabled: {}", i, items_info_[i]->info_->name(), items_info_[i]->info_->enabled());
                QListWidgetItem *item = stream_list_->item(i);
                auto item_widget = (StPluginItemWidget*)stream_list_->itemWidget(item);
                item_widget->UpdateStatus(items_info_[i]);
            }
        });
    }

}
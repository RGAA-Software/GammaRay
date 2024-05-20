//
// Created by RGAA on 2024-04-09.
//

#include "tab_game.h"
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <boost/format.hpp>
#include <utility>

#include "db/db_game.h"
#include "db/db_game_manager.h"
#include "gr_context.h"
#include "app_messages.h"
#include "widgets/main_item_delegate.h"
#include "widgets/round_img_display.h"
#include "widgets/cover_widget.h"
#include "widgets/layout_helper.h"
#include "tc_common_new/log.h"
#include "widget_decorator.h"
#include "tc_common_new/task_runtime.h"
#include "tc_common_new/message_notifier.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_steam_manager_new/steam_entities.h"

namespace tc
{

    TabGame::TabGame(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent) {
        steam_mgr_ = context_->GetSteamManager();
        auto root_layout = new QVBoxLayout();
        LayoutHelper::ClearMargins(root_layout);

        list_widget_ = new QListWidget(this);
        //list_widget_->verticalScrollBar()->setStyleSheet(Style::GetScrollBar().c_str());

        auto delegate = new MainItemDelegate(this);
        list_widget_->setItemDelegate(delegate);

        root_layout->addSpacing(10);
        root_layout->addWidget(list_widget_);

        list_widget_->setMovement(QListView::Static);
        list_widget_->setViewMode(QListView::IconMode);
        list_widget_->setFlow(QListView::LeftToRight);
        list_widget_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list_widget_->setSpacing(10);
        list_widget_->setResizeMode(QListWidget::Adjust);
        boost::format format(R"(
            QListWidget { outline: none; border:0px solid gray; color: black; background-color: none; padding-left: %1%px; padding-right: %2%px; padding-bottom: %3%px;}
        )");
        format% (25);
        format% (20);
        format% (20);

        //
        QObject::connect(list_widget_, &QListWidget::clicked, this, [=, this](const QModelIndex& index) {
            int row = index.row();
            if (row >= this->games_.size()) {
                return;
            }
            LOGI("click: {}", row);
        });

        QObject::connect(list_widget_, &QListWidget::doubleClicked, this, [=, this](const QModelIndex& index) {
            int row = index.row();
            if (row >= this->games_.size()) {
                return;
            }
            LOGI("double click: {}", row);
        });

        list_widget_->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(list_widget_, &QListWidget::customContextMenuRequested, this, [=, this](const QPoint& pos) {
            QListWidgetItem* cur_item = list_widget_->itemAt(pos);
            if (cur_item == nullptr) {
                return;
            }

            int index = list_widget_->row(cur_item);
            if (index < 0 || index >= games_.size()) {
                return;
            }
            auto wp_entity = games_.at(index);

            std::vector<QString> actions {
                tr("11111111"),
                tr("33333333"),
                tr("55555555"),
                tr("77777777"),
            };
            auto pop_menu = new QMenu();
            //pop_menu->setFont(sk::SysConfig::Instance()->sys_font_9);
            for (int i = 0; i < actions.size(); i++) {
                auto action = new QAction(actions.at(i), pop_menu);
                //action->setFont(sk::SysConfig::Instance()->sys_font_9);
                WidgetDecorator::DecoratePopMenu(pop_menu);
                pop_menu->addAction(action);

                QObject::connect(action, &QAction::triggered, this, [=]() {

                });
            }

            pop_menu->exec(QCursor::pos());
            delete pop_menu;

        });

        list_widget_->setStyleSheet(format.str().c_str());
        list_widget_->show();

        setLayout(root_layout);

        // listeners
        msg_listener_->Listen<MsgRunningGameIds>([=, this](const MsgRunningGameIds& rgs) {
            LOGI("Running ids: ");
            for (auto& gid : rgs.game_ids_) {
                LOGI("game id: {}", gid);
            }
        });

        ScanInstalledGames();
    }

    TabGame::~TabGame() {

    }

    void TabGame::OnTabShow() {

    }

    void TabGame::OnTabHide() {

    }

    void TabGame::ScanInstalledGames() {
        context_->PostTask([=, this]() {
            // 1. load from database
            auto gm = context_->GetDBGameManager();
            games_ = gm->GetAllGames();
            if (!games_.empty()) {
                for (auto& game : games_) {
                    this->LoadCover(game);
                }
                AddItems(games_);
            } else {
                // todo: 1.loading ....
            }

            steam_mgr_->ScanInstalledGames();
            steam_mgr_->DumpGamesInfo();

            auto steam_apps = steam_mgr_->GetInstalledGames();
            std::vector<TcDBGamePtr> scan_games;

            for (auto& app : steam_apps) {
                auto game = std::make_shared<TcDBGame>();
                game->CopyFrom(app);
                scan_games.push_back(game);
            }
            gm->BatchSaveOrUpdateGames(scan_games);

            if (games_.empty() && scan_games.empty()) {
                // todo 2.show empty
            } else {
                // todo 3.hide loading
            }

            if (games_.empty()) {
                games_ = scan_games;
                for (auto& game : games_) {
                    this->LoadCover(game);
                }
                AddItems(games_);
            }

        });
    }

    QListWidgetItem* TabGame::AddItem(int index, const TcDBGamePtr& game) {
        auto item = new QListWidgetItem(list_widget_);
        int margin = 0;
        auto item_size = GetItemSize();
        int item_width = item_size.width();
        int item_height = item_size.height();
        item->setSizeHint(QSize(item_width, item_height));

        auto widget = new QWidget(this);
        widget->setFixedSize(item_width, item_height);

        auto layout = new QVBoxLayout();
        LayoutHelper::ClearMargins(layout);
        auto cover = new RoundImageDisplay("", item_width, item_height, 9, widget);
        cover->setObjectName("cover");
        layout->addWidget(cover);

        if (game->cover_pixmap_.has_value()) {
            auto pixmap = std::any_cast<QPixmap>(game->cover_pixmap_);
            cover->UpdatePixmap(pixmap);
        }

        widget->setLayout(layout);
        widget->show();

        auto cover_widget = new CoverWidget(widget, 0);
        cover_widget->setObjectName("cover_mask");
        cover_widget->setFixedSize(item_width, item_height);

        auto name = new QLabel(widget);
        name->setFixedSize(item_width, 32);
        name->setStyleSheet(R"(background-color:#333333; border-radius: 7px; color:#ffffff;)");
        name->setAlignment(Qt::AlignCenter);
        name->setGeometry(0, item_height-name->height(), item_width, name->height());
        name->setText(game->game_name_.c_str());

        LOGI("engine type: {}", game->engine_type_);
        if (game->engine_type_ != "UNKNOWN") {
            auto engine = new QLabel(widget);
            engine->setFixedSize(60, 22);
            engine->setText(game->engine_type_.c_str());
            engine->setStyleSheet(R"(background-color:#333333; border-radius: 11px; color:#ffffff; font-size:10px;)");
            engine->setAlignment(Qt::AlignCenter);
            engine->setGeometry(item_width - engine->width() - 5, 5, engine->width(), engine->height());
        }

        list_widget_->setItemWidget(item, widget);
        return item;
    }

    QSize TabGame::GetItemSize() {
        int item_width = 180;
        int item_height = item_width / (600.0/900.0);
        return QSize(item_width, item_height);
    }

    void TabGame::AddItems(const std::vector<TcDBGamePtr>& games) {
        context_->PostUITask([=, this]() {
            for (int i = 0; i < games.size(); i++) {
                AddItem(i, games[i]);
            }
        });
    }

    void TabGame::LoadCover(const tc::TcDBGamePtr &game) {
        QImage image;
        if (!image.load(QString::fromStdString(game->cover_url_))) {
            LOGI("not find cover: {}", game->cover_url_);
            return;
        }
        auto item_size = GetItemSize();
        auto pixmap = QPixmap::fromImage(image);
        pixmap = pixmap.scaled(item_size.width(), item_size.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        game->cover_pixmap_ = pixmap;
    }

}
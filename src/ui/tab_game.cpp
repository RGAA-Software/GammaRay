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

#include "context.h"
#include "widgets/main_item_delegate.h"
#include "widgets/round_img_display.h"
#include "widgets/cover_widget.h"
#include "tc_common_new/task_runtime.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_steam_manager_new/steam_entities.h"
#include "widgets/layout_helper.h"

namespace tc
{

    TabGame::TabGame(const std::shared_ptr<Context>& ctx, QWidget* parent) : TabBase(ctx, parent) {
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

        list_widget_->setStyleSheet(format.str().c_str());
        list_widget_->show();

        setLayout(root_layout);

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
            steam_mgr_->ScanInstalledGames();
            steam_mgr_->DumpGamesInfo();

            auto games = steam_mgr_->GetInstalledGames();
            for (auto& game : games) {
                QImage image;
                if (!image.load(QString::fromStdString(game->cover_url_))) {
                    continue;
                }
                auto pixmap = QPixmap::fromImage(image);
                pixmap = pixmap.scaled(200, 200/ (600.0/900.0), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                game->cover_pixmap_ = pixmap;
            }

            context_->PostUITask([=, this]() {
                for (int i = 0; i < games.size(); i++) {
                    AddItem(i, games[i]);
                }
            });
        });
    }

    QListWidgetItem* TabGame::AddItem(int index, const std::shared_ptr<SteamApp>& game) {
        auto item = new QListWidgetItem(list_widget_);
        int margin = 0;
        int item_width = 200;
        int item_height = item_width / (600.0/900.0);
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

        list_widget_->setItemWidget(item, widget);
        return item;
    }
}
//
// Created by RGAA  on 2024/5/20.
//

#include "game_info_preview.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_qt_widget/round_img_display.h"
#include "render_panel/database/db_game.h"

namespace tc
{

    GameInfoPreview::GameInfoPreview(const std::shared_ptr<GrApplication>& app, const std::shared_ptr<TcDBGame>& game, QWidget* parent)
        : QDialog(parent) {

        this->setWindowTitle(tr("Game Info"));

        auto root_layout = new NoMarginHLayout();
        auto left_layout = new NoMarginVLayout();
        // image
        {
            int item_width = 135;
            int item_height = item_width / (600.0 / 900.0);
            auto cover = new RoundImageDisplay("", item_width, item_height, 9, this);
            if (game->cover_pixmap_.has_value()) {
                auto pixmap = std::any_cast<QPixmap>(game->cover_pixmap_);
                pixmap = pixmap.scaled(item_width, item_height);
                cover->UpdatePixmap(pixmap);
            }
            left_layout->addSpacing(20);
            left_layout->addWidget(cover);
            left_layout->addStretch();
        }
        root_layout->addSpacing(20);
        root_layout->addLayout(left_layout);

        // info
        auto info_layout = new NoMarginVLayout();
        info_layout->addSpacing(20);
        // name
        {
            auto label = new QLabel(this);
            label->setText(game->game_name_.c_str());
            label->setStyleSheet("font-weight:700; font-size:16px;");
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
            label->setCursor(Qt::IBeamCursor);
            info_layout->addWidget(label);
        }
        {
            auto label = new QLabel(this);
            label->setText(std::to_string(game->game_id_).c_str());
            label->setStyleSheet("font-weight:500; font-size:14px;");
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
            label->setCursor(Qt::IBeamCursor);
            info_layout->addSpacing(8);
            info_layout->addWidget(label);
        }
        {
            auto label = new QLabel(this);
            label->setText(game->steam_url_.c_str());
            label->setStyleSheet("font-weight:500; font-size:14px;");
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
            label->setCursor(Qt::IBeamCursor);
            info_layout->addSpacing(8);
            info_layout->addWidget(label);
        }
        {
            auto label = new QLabel(this);
            label->setWordWrap(true);
            label->setText(game->game_installed_dir_.c_str());
            label->setStyleSheet("font-weight:500; font-size:14px;");
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
            label->setCursor(Qt::IBeamCursor);
            info_layout->addSpacing(8);
            info_layout->addWidget(label);
        }
        info_layout->addStretch();
        root_layout->addSpacing(20);
        root_layout->addLayout(info_layout);
        root_layout->addSpacing(20);

        //root_layout->addStretch();
        setLayout(root_layout);
    }


}
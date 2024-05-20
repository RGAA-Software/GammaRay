//
// Created by hy on 2024/5/20.
//

#include "game_info_preview.h"
#include "widgets/no_margin_layout.h"
#include "widgets/round_img_display.h"

namespace tc
{

    GameInfoPreview::GameInfoPreview(const std::shared_ptr<GrApplication>& app, const std::shared_ptr<TcDBGame>& game, QWidget* parent)
        : QDialog(parent) {
        auto root_layout = new NoMarginHLayout();

        auto left_layout = new NoMarginVLayout();
        // image
        {
            int item_width = 135;
            int item_height = item_width / (600.0 / 900.0);
            auto cover = new RoundImageDisplay("", item_width, item_height, 9, this);
            left_layout->addWidget(cover);
            left_layout->addStretch();
        }
        root_layout->addLayout(left_layout);

        root_layout->addStretch();
        setLayout(root_layout);
    }


}
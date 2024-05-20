//
// Created by hy on 2024/5/20.
//

#ifndef GAMMARAY_GAME_INFO_PREVIEW_H
#define GAMMARAY_GAME_INFO_PREVIEW_H

#include <QWidget>
#include <QDialog>

namespace tc
{

    class TcDBGame;
    class GrApplication;

    class GameInfoPreview : public QDialog {
    public:

        GameInfoPreview(const std::shared_ptr<GrApplication>& app, const std::shared_ptr<TcDBGame>& game, QWidget* parent = nullptr);
        ~GameInfoPreview() = default;

    };

}

#endif //GAMMARAY_GAME_INFO_PREVIEW_H

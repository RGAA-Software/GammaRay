//
// Created by hy on 2024/6/21.
//

#ifndef GAMMARAY_ADD_GAME_PANEL_H
#define GAMMARAY_ADD_GAME_PANEL_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>

namespace tc
{

    class GrContext;

    class AddGamePanel : public QDialog {
    public:
        AddGamePanel(const std::shared_ptr<GrContext>& ctx, QWidget* parent);
    private:
        void SaveGame();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        QLineEdit* lbl_game_name_ = nullptr;
        QLabel* lbl_game_installed_dir_ = nullptr;
        QLabel* lbl_game_exe_name_ = nullptr;
        QLineEdit* edit_game_exe_path_ = nullptr;
        QString cover_path_;
        QString cover_name_;
        QLabel* cover_preview_;
    };

}

#endif //GAMMARAY_ADD_GAME_PANEL_H

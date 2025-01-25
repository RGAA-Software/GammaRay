//
// Created by RGAA  on 2024/6/21.
//

#include "add_game_panel.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/log.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "render_panel/db/db_game.h"
#include "render_panel/db/db_game_manager.h"
#include "gr_context.h"
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QGridLayout>
#include <QFileDialog>
#include <QApplication>

namespace tc
{

    AddGamePanel::AddGamePanel(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : QDialog(parent) {
        context_ = ctx;
        setWindowTitle(tr("Add a game"));
        setFixedSize(720, 480);
        setModal(true);

        auto layout = new NoMarginVLayout();

        auto item_layout = new QGridLayout();
        item_layout->setSpacing(10);
        layout->addSpacing(10);
        layout->addLayout(item_layout);

        auto label_size = QSize(150, 30);
        auto edit_size = QSize(300, 30);
        // name
        {
            auto label = new QLabel(this);
            label->setText(tr("Game name"));
            label->setFixedSize(label_size);
            item_layout->addWidget(label, 0, 0);

            auto edit = new QLineEdit(this);
            lbl_game_name_ = edit;
            edit->setFixedSize(edit_size);
            item_layout->addWidget(edit, 0, 1);
        }
        // exe path
        {
            auto label = new QLabel(this);
            label->setText(tr("Game exe path"));
            label->setFixedSize(label_size);
            item_layout->addWidget(label, 1, 0);

            auto edit = new QLineEdit(this);
            edit->setFixedSize(edit_size);
            item_layout->addWidget(edit, 1, 1);
            edit_game_exe_path_ = edit;

            auto btn = new QPushButton(this);
            btn->setFixedSize(120, 30);
            btn->setText(tr("Select"));
            item_layout->addWidget(btn, 1, 2);

            connect(btn, &QPushButton::clicked, this, [=, this]() {
                auto filepath = QFileDialog::getOpenFileName(this, tr("Select game exe"), "/home", tr("exe (*.exe)"));
                if (filepath.isEmpty()) {
                    return;
                }
                QFile file(filepath);
                if (!file.exists()) {
                    return;
                }
                edit->setText(filepath);

                QFileInfo fileinfo(file);
                auto filename = fileinfo.fileName();
                auto file_folder = fileinfo.absolutePath();
                lbl_game_exe_name_->setText(filename);
                lbl_game_installed_dir_->setText(file_folder);
                LOGI("filename: {}, path: {}", filename.toStdString(), file_folder.toStdString());
            });
        }

        {
            auto label = new QLabel(this);
            label->setText(tr("Game exe"));
            label->setFixedSize(label_size);
            item_layout->addWidget(label, 2, 0);

            auto info = new QLabel(this);
            lbl_game_exe_name_ = info;
            info->setFixedSize(edit_size);
            item_layout->addWidget(info, 2, 1);
        }

        {
            auto label = new QLabel(this);
            label->setText(tr("Game installed dir"));
            label->setFixedSize(label_size);
            item_layout->addWidget(label, 3, 0);

            auto info = new QLabel(this);
            lbl_game_installed_dir_ = info;
            info->setWordWrap(true);
            info->setFixedSize(edit_size.width(), edit_size.height()*3);
            item_layout->addWidget(info, 3, 1);
        }

        // cover
        {
            auto label = new QLabel(this);
            label->setText(tr("Game cover(600x900)"));
            label->setFixedSize(label_size);
            item_layout->addWidget(label, 4, 0);

            auto edit = new QLabel(this);
            edit->setFixedSize(QSize(100, 150));
            edit->setStyleSheet("background:#999999;");
            cover_preview_ = edit;
            item_layout->addWidget(edit, 4, 1);

            auto btn = new QPushButton(this);
            btn->setFixedSize(120, 30);
            btn->setText(tr("Select"));
            item_layout->addWidget(btn, 4, 2);

            connect(btn, &QPushButton::clicked, this, [=, this]() {
                auto filepath = QFileDialog::getOpenFileName(this, tr("Select game exe"), "/home", tr("Images (*.jpg *.png)"));
                if (filepath.isEmpty()) {
                    return;
                }
                QFile file(filepath);
                if (!file.exists()) {
                    return;
                }

                QImage image;
                image.load(filepath);
                auto pixmap = QPixmap::fromImage(image);
                if (!pixmap.isNull()) {
                    pixmap = pixmap.scaled(edit->width(), edit->height());
                    cover_preview_->setPixmap(pixmap);
                }

                auto res_folder = qApp->applicationDirPath() + "/www";
                LOGI("res folder: {}", res_folder.toStdString());
                QDir static_dir;
                if (!static_dir.exists(res_folder)) {
                    if (!static_dir.mkdir(res_folder)) {
                        return;
                    }
                }

                QFileInfo file_info(file);
                auto target_name = res_folder + "/" + file_info.fileName();
                if (QFile::exists(target_name)) {
                    QFile::remove(target_name);
                }
                if (file.copy(target_name)) {
                    cover_path_ = target_name;
                    cover_name_ = file_info.fileName();
                }
            });
        }

        layout->addStretch();

        auto op_layout = new NoMarginHLayout();
        op_layout->addStretch();
        {
            auto btn = new QPushButton(this);
            btn->setText(tr("Cancel"));
            btn->setFixedSize(120, 30);
            btn->setProperty("class", "danger");
            op_layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                done(1);
            });
        }
        {
            auto btn = new QPushButton(this);
            btn->setText(tr("Sure"));
            btn->setFixedSize(120, 30);
            op_layout->addSpacing(20);
            op_layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                SaveGame();
            });
        }
        op_layout->addStretch();
        layout->addLayout(op_layout);
        layout->addSpacing(40);
        setLayout(layout);
    }

    void AddGamePanel::SaveGame() {
        if (lbl_game_name_->text().isEmpty()
            || lbl_game_exe_name_->text().isEmpty()
            || lbl_game_installed_dir_->text().isEmpty()) {
            SizedMessageBox::MakeErrorOkBox(tr("Error Info"), tr("Please input valid info"))->exec();
            return;
        }

        auto func_gen_id = [](const std::string& md5) -> uint32_t {
            std::string sub = md5.substr(0, 16);
            std::istringstream converter(sub);
            unsigned long long result;
            converter >> std::hex >> result;
            return result % 99999999;
        };

        auto game = std::make_shared<TcDBGame>();
        game->game_name_ = lbl_game_name_->text().toStdString();
        game->game_exes_ = edit_game_exe_path_->text().toStdString();
        game->game_exe_names_ = lbl_game_exe_name_->text().toStdString();
        game->game_installed_dir_ = lbl_game_installed_dir_->text().toStdString();
        game->cover_url_ = cover_path_.toStdString();
        game->game_id_ = func_gen_id(MD5::Hex(game->game_exes_));
        game->cover_name_ = cover_name_.toStdString();
        auto game_mgr = context_->GetDBGameManager();
        game_mgr->SaveOrUpdateGame(game);
        done(0);
        SizedMessageBox::MakeOkBox(tr("Success"), tr("Save game success !"))->exec();
    }
}
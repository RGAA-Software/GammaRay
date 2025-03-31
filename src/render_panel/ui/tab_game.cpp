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
#include <QStyledItemDelegate>
#include <utility>
#include "shellapi.h"
#include "render_panel/db/db_game.h"
#include "render_panel/db/db_game_manager.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "tc_qt_widget/round_img_display.h"
#include "tc_qt_widget/cover_widget.h"
#include "tc_qt_widget/widget_helper.h"
#include "tc_common_new/log.h"
#include "tc_common_new/task_runtime.h"
#include "tc_common_new/message_notifier.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_steam_manager_new/steam_entities.h"
#include "game_info_preview.h"
#include "tc_common_new/folder_util.h"
#include "render_panel/gr_run_game_manager.h"
#include "tc_common_new/process_util.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "add_game_panel.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "tc_common_new/folder_util.h"
#include "tc_common_new/file_util.h"
#include "tc_dialog.h"

namespace tc
{

    class MainItemDelegate : public QStyledItemDelegate {
    public:
        explicit MainItemDelegate(QObject *pParent) : QStyledItemDelegate(pParent) {}
        ~MainItemDelegate() override {}
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override {
            editor->setGeometry(option.rect);
        }
    };

    TabGame::TabGame(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent) {
        steam_mgr_ = context_->GetSteamManager();
        auto root_layout = new QVBoxLayout();
        WidgetHelper::ClearMargins(root_layout);

        // operators
        auto op_layout = new NoMarginHLayout();
        auto btn_size = QSize(120, 33);
        root_layout->addSpacing(10);
        {
            auto btn = new QPushButton(this);
            btn->setText(tr("Add Game"));
            btn->setFixedSize(btn_size);
            op_layout->addSpacing(15);
            op_layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                ShowAddGamePanel();
            });
        }
        {
            auto btn = new QPushButton(this);
            btn->setText(tr("Refresh"));
            btn->setFixedSize(btn_size);
            op_layout->addSpacing(15);
            op_layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                RefreshGames();
            });
        }
        op_layout->addStretch();
        root_layout->addLayout(op_layout);

        list_widget_ = new QListWidget(this);
        auto delegate = new MainItemDelegate(this);
        list_widget_->setItemDelegate(delegate);

        root_layout->addWidget(list_widget_);

        list_widget_->setMovement(QListView::Static);
        list_widget_->setViewMode(QListView::IconMode);
        list_widget_->setFlow(QListView::LeftToRight);
        list_widget_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list_widget_->setSpacing(10);
        list_widget_->setResizeMode(QListWidget::Adjust);
        //
        QObject::connect(list_widget_, &QListWidget::clicked, this, [=, this](const QModelIndex& index) {
            int row = index.row();
            if (row >= this->games_.size()) {
                return;
            }
        });

        QObject::connect(list_widget_, &QListWidget::doubleClicked, this, [=, this](const QModelIndex& index) {
            int row = index.row();
            if (row >= this->games_.size()) {
                return;
            }
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
            auto game = games_.at(index);

            std::vector<QString> actions {
                tr("Game Info"),
                tr("Start Game"),
                tr("Stop Game"),
                tr("Installed Location"),
            };
            auto pop_menu = new QMenu();
            for (int i = 0; i < actions.size(); i++) {
                auto action = new QAction(actions.at(i), pop_menu);
                pop_menu->addAction(action);

                if (i == 0) {
                    QObject::connect(action, &QAction::triggered, this, [=, this]() {
                        GameInfoPreview preview(app_, game);
                        preview.setFixedSize(640, 480);
                        preview.exec();
                    });
                } else if (i == 1) {
                    QObject::connect(action, &QAction::triggered, this, [=, this]() {
                        if (!game->steam_url_.empty()) {
                            ShellExecuteA(nullptr, nullptr, game->steam_url_.c_str(), nullptr, nullptr, SW_SHOW);
                        } else {
                            auto func_start_error = [this](const std::string& msg) {
                                //SizedMessageBox::MakeErrorOkBox(tr("Error"), std::format("Start process failed: {}", msg).c_str())->exec();

                                auto dlg = TcDialog::Make(tr("Error"), std::format("Start process failed: {}", msg).c_str(), nullptr);
                                dlg->SetOnDialogSureClicked([=, this]() {});
                                dlg->show();
                            };

                            if (game->exes_.empty()) {
                                func_start_error("Don't have exe");
                                return;
                            }
                            auto exe_path = game->exes_.at(0);
                            if (!QFile::exists(exe_path.c_str())) {
                                func_start_error(std::format("File not exist: {}", exe_path));
                                return;
                            }

                            context_->PostTask([=, this]() {
                                LOGI("Will start: {}", exe_path);
                                auto resp = context_->GetRunGameManager()->StartGame(exe_path, {});
                                if (!resp.ok_) {
                                    func_start_error("start error");
                                }
                            });
                        }
                    });
                } else if (i == 2) {
                    QObject::connect(action, &QAction::triggered, this, [=, this]() {
                        this->context_->PostTask([=, this]() {
                            auto rgm = this->context_->GetRunGameManager();
                            auto running_games = rgm->GetRunningGames();
                            for (const auto& rg : running_games) {
                                if (rg->game_->game_id_ != game->game_id_) {
                                    continue;
                                }
                                for (auto pid : rg->pids_) {
                                    ProcessUtil::KillProcess(pid);
                                }
                            }
                        });
                    });
                } else if (i == 3) {
                    QObject::connect(action, &QAction::triggered, this, [=, this]() {
                        FolderUtil::OpenDir(game->game_installed_dir_);
                    });
                }
            }

            pop_menu->exec(QCursor::pos());
            delete pop_menu;

        });

        list_widget_->setStyleSheet("QListWidget {background-color:none;}");
        list_widget_->show();

        setLayout(root_layout);

        // listeners
        msg_listener_->Listen<MsgRunningGameIds>([=, this](const MsgRunningGameIds& rgs) {
            this->context_->PostUITask([=, this]() {
                this->UpdateRunningStatus(rgs.game_ids_);
            });
        });

        ScanInstalledGames();
    }

    TabGame::~TabGame() = default;

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

            steam_mgr_->ScanInstalledGames(false);
            //steam_mgr_->DumpGamesInfo();

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

            context_->PostTask([=, this]() {
                steam_mgr_->RescanRecursively();
            });

        });

    }

    QListWidgetItem* TabGame::AddItem(const TcDBGamePtr& game) {
        auto item = new QListWidgetItem(list_widget_);
        int margin = 0;
        auto item_size = GetItemSize();
        int item_width = item_size.width();
        int item_height = item_size.height();
        item->setSizeHint(QSize(item_width, item_height));

        auto widget = new QWidget(this);
        widget->setFixedSize(item_width, item_height);
        widget->setObjectName(std::to_string(game->game_id_).c_str());

        auto layout = new QVBoxLayout();
        WidgetHelper::ClearMargins(layout);
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
        name->update();
        name->show();

        LOGI("engine type: {}", game->engine_type_);
        if (game->engine_type_ != "UNKNOWN" && !game->engine_type_.empty()) {
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
        return {item_width, item_height};
    }

    void TabGame::AddItems(const std::vector<TcDBGamePtr>& games) {
        context_->PostUITask([=, this]() {
            for (const auto& game : games) {
                AddItem(game);
            }
        });
    }

    void TabGame::LoadCover(const tc::TcDBGamePtr& game) {
        auto steam_manager = context_->GetSteamManager();
        if (!steam_manager) {
            return;
        }
        auto image_cache_path = steam_manager->GetSteamImageCachePath();
        std::string from_cover_path = std::format("{}/{}/library_600x900.jpg", image_cache_path, game->game_id_);
        LOGI("cover path: {}", from_cover_path);

        std::string steam_cover_folder_path = this->context_->GetCurrentExeFolder() + "/resources/steam_covers";
        FolderUtil::CreateDir(steam_cover_folder_path);
        std::string target_cover_name = std::format("{}_library_600x900.jpg", game->game_id_);
        std::string target_cover_path = std::format("{}/{}", steam_cover_folder_path, target_cover_name);
        FileUtil::CopyFileExt(from_cover_path, target_cover_path, false);

        QImage image;
        if (!image.load(QString::fromStdString(target_cover_path))) {
            LOGI("not find cover: {}", target_cover_path);
            return;
        }
        auto item_size = GetItemSize();
        auto pixmap = QPixmap::fromImage(image);
        pixmap = pixmap.scaled(item_size.width(), item_size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        game->cover_pixmap_ = pixmap;
    }

    void TabGame::UpdateRunningStatus(const std::vector<uint64_t>& game_ids) {
        this->VisitListWidget([=, this](QListWidgetItem* item, QWidget* item_widget) {
            auto cover_widget = item_widget->findChild<CoverWidget*>("cover_mask");
            cover_widget->SetRunningStatus(false);
        });
        this->VisitListWidget([=, this](QListWidgetItem* item, QWidget* item_widget) {
            auto cover_widget = item_widget->findChild<CoverWidget*>("cover_mask");
            auto game_id = item_widget->objectName().toStdString();
            for (auto rgid : game_ids) {
                if (std::to_string(rgid) == game_id) {
                    LOGI("running game id: {}", rgid);
                    cover_widget->SetRunningStatus(true);
                }
            }
        });
    }

    void TabGame::VisitListWidget(std::function<void(QListWidgetItem* item, QWidget* item_widget)>&& cbk) {
        auto item_counts = list_widget_->count();
        for (int i = 0; i < item_counts; i++) {
            QListWidgetItem *item = list_widget_->item(i);
            auto item_widget = list_widget_->itemWidget(item);
            cbk(item, item_widget);
        }
    }

    void TabGame::ShowAddGamePanel() {
        AddGamePanel panel(context_, this);
        panel.exec();
        RefreshGames();
    }

    void TabGame::RefreshGames() {
        context_->PostTask([=, this]() {
            auto db_mgr = context_->GetDBGameManager();
            auto rescan_games = db_mgr->GetAllGames();
            for (auto& rg : rescan_games) {
                bool find = false;
                for (auto& exist_game : games_) {
                    if (exist_game->game_id_ == rg->game_id_) {
                        find = true;
                        break;
                    }
                }

                if (!find) {
                    context_->PostUITask([=, this]() {
                        QImage image;
                        image.load(rg->cover_url_.c_str());
                        auto pixmap = QPixmap::fromImage(image);
                        auto item_size = GetItemSize();
                        pixmap = pixmap.scaled(item_size.width(), item_size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                        if (!pixmap.isNull()) {
                            rg->cover_pixmap_ = pixmap;
                        }
                        AddItem(rg);
                    });
                    games_.push_back(rg);
                }
            }
        });
    }

}
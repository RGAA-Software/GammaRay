//
// Created by RGAA on 2024/4/9.
//

#include "gr_workspace.h"
#include "gr_application.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QApplication>
#include <QMenu>

#include "tc_qt_widget/custom_tab_btn.h"
#include "tc_qt_widget/layout_helper.h"
#include "render_panel/ui/tab_game.h"
#include "render_panel/ui/tab_server.h"
#include "render_panel/ui/tab_settings.h"
#include "gr_settings.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "gr_context.h"
#include "gr_render_controller.h"
#include "service/service_manager.h"
#include "app_colors.h"

namespace tc
{

    GrWorkspace::GrWorkspace() : QMainWindow(nullptr) {
        setWindowTitle(tr("GammaRay"));
        settings_ = GrSettings::Instance();

        auto menu = new QMenu(this);
        sys_tray_icon_ = new QSystemTrayIcon(this);
        sys_tray_icon_->setIcon(QIcon(":/resources/tc_icon.png"));
        sys_tray_icon_->setToolTip(tr("GammaRay"));

        auto ac_show = new QAction(tr("Show Panel"), this);
        auto ac_exit = new QAction(tr("Exit Program"), this);

        connect(ac_show, &QAction::triggered, this, [=, this](bool) {
            this->showNormal();
        });

        auto fun_stop_all = [=, this]() {
            auto msg_box = SizedMessageBox::MakeOkCancelBox(tr("Exit"), tr("Do you want to exit program?"));
            if (msg_box->exec() == 0) {
                auto srv_mgr = this->app_->GetContext()->GetServiceManager();
                srv_mgr->Remove();
            }
        };

        connect(ac_exit, &QAction::triggered, this, [=, this](bool) {
            fun_stop_all();
        });

        menu->addAction(ac_show);
        menu->addAction(ac_exit);
        sys_tray_icon_->setContextMenu(menu);
        sys_tray_icon_->show();
        connect(sys_tray_icon_, &QSystemTrayIcon::activated, this, [=, this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::ActivationReason::DoubleClick) {
                this->show();
            }
        });

        theme_ = new MainWindowPrivate(this);
        QString app_dir = qApp->applicationDirPath();
        QString style_dir = app_dir + "/resources/";
        theme_->AdvancedStyleSheet = new acss::QtAdvancedStylesheet(this);
        theme_->AdvancedStyleSheet->setStylesDirPath(style_dir);
        theme_->AdvancedStyleSheet->setOutputDirPath(app_dir + "/output");
        theme_->AdvancedStyleSheet->setCurrentStyle("qt_material");
        theme_->AdvancedStyleSheet->setCurrentTheme("light_blue");
        theme_->AdvancedStyleSheet->updateStylesheet();
        setWindowIcon(theme_->AdvancedStyleSheet->styleIcon());
        qApp->setStyleSheet(theme_->AdvancedStyleSheet->styleSheet());

        app_ = std::make_shared<GrApplication>();
        app_->Init();

        // background
        setStyleSheet(R"(QMainWindow {background-color:#FFFFFF;})");

        // root
        auto root_layout = new QHBoxLayout();
        LayoutHelper::ClearMargins(root_layout);

        // left buttons
        {
            auto layout = new QVBoxLayout();
            LayoutHelper::ClearMargins(layout);

            // placeholder to extend the width of left area
            int left_area_width = 220;
            auto extend = new QLabel(this);
            extend->setFixedSize(left_area_width, 2);
            layout->addWidget(extend);

            // logo
            {
                auto logo = new QLabel(this);
                int logo_size = 90;
                logo->setFixedSize(logo_size, logo_size);
                QImage image;
                image.load(":/resources/tc_icon.png");
                auto pixmap = QPixmap::fromImage(image);
                pixmap = pixmap.scaled(logo_size, logo_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                logo->setPixmap(pixmap);
                layout->addSpacing(45);
                layout->addWidget(logo, 0, Qt::AlignHCenter);
            }

            // buttons
            auto btn_font_color = "#ffffff";
            auto btn_size = QSize(left_area_width - 30, 36);
            {
                auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
                btn_tab_server_ = btn;
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetText(tr("Server"));
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    ChangeTab(TabName::kTabServer);
                });
                layout->addSpacing(30);
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            {
                auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
                btn_tab_games_ = btn;
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetText(tr("Games"));
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    ChangeTab(TabName::kTabGames);
                });
                layout->addSpacing(10);
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            {
                auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
                btn_tab_settings_ = btn;
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetText(tr("Settings"));
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    ChangeTab(TabName::kTabSettings);
                });
                layout->addSpacing(10);
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            layout->addStretch();

            // stop all
            {
                auto btn = new QPushButton(this);
                btn->setText(tr("Exit Program"));
                btn->setProperty("class", "danger");
                btn->setProperty("flat", true);
                btn->setFixedSize(btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    fun_stop_all();
                });
                layout->addWidget(btn, 0, Qt::AlignHCenter);
                layout->addSpacing(15);
            }

            // version
            {
                auto label = new QLabel(this);
                label->setText(settings_->version_.c_str());
                layout->addWidget(label, 0, Qt::AlignHCenter);
                layout->addSpacing(10);
            }

            root_layout->addLayout(layout);
        }

        // right panels
        {
            // tabs
            tabs_.insert({TabName::kTabServer, new TabServer(app_, this)});
            tabs_.insert({TabName::kTabGames, new TabGame(app_, this)});
            tabs_.insert({TabName::kTabSettings, new TabSettings(app_, this)});

            tabs_[TabName::kTabServer]->SetAttach(btn_tab_server_);
            tabs_[TabName::kTabGames]->SetAttach(btn_tab_games_);
            tabs_[TabName::kTabSettings]->SetAttach(btn_tab_settings_);

            auto layout = new QVBoxLayout();
            LayoutHelper::ClearMargins(root_layout);
            auto stack_widget = new QStackedWidget(this);
            stack_widget->addWidget(tabs_[TabName::kTabServer]);
            stack_widget->addWidget(tabs_[TabName::kTabGames]);
            stack_widget->addWidget(tabs_[TabName::kTabSettings]);
            stacked_widget_ = stack_widget;
            layout->addWidget(stack_widget);
            root_layout->addLayout(layout);
        }

        auto root_widget = new QWidget(this);
        root_widget->setLayout(root_layout);
        setCentralWidget(root_widget);

        ChangeTab(TabName::kTabServer);

        // adjust window's position
        QTimer::singleShot(50, [this]() {
            auto screen = qApp->primaryScreen();
            if (!screen) {return;}

            auto screen_size = screen->size();
            auto x = (screen_size.width() - this->size().width())/2;
            auto y = (screen_size.height() - this->size().height() - 48)/2;
            this->move(x, y);
        });
    }

    void GrWorkspace::ChangeTab(const TabName& tn) {
        for (auto& [name, tab] : tabs_) {
            if (tn == name) {
                stacked_widget_->setCurrentWidget(tabs_[tn]);
                tabs_[tn]->OnTabShow();
                ((CustomTabBtn*)tabs_[tn]->GetAttach())->ToActiveStatus();
            } else {
                tabs_[name]->OnTabShow();
                ((CustomTabBtn*)tabs_[name]->GetAttach())->ToInActiveStatus();
            }
        }
    }

    void GrWorkspace::closeEvent(QCloseEvent *event) {
        auto dlg = SizedMessageBox::MakeOkCancelBox(tr("Hide"), tr("Do you want to hide GammaRay?"));
        if (dlg->exec() == 0) {
            this->hide();
            event->ignore();
        } else {
            event->ignore();
        }
    }

    void GrWorkspace::resizeEvent(QResizeEvent *event) {
        QMainWindow::resizeEvent(event);
    }

}
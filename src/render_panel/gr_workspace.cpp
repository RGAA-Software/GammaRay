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
#include <dwmapi.h>
#include <QSvgRenderer>
#include <QPainter>
#include <QStandardPaths>

#include "tc_qt_widget/custom_tab_btn.h"
#include "tc_qt_widget/widget_helper.h"
#include "render_panel/ui/tab_game.h"
#include "render_panel/ui/tab_server.h"
#include "render_panel/ui/tab_settings.h"
#include "render_panel/ui/tab_profile.h"
#include "render_panel/ui/tab_security_internals.h"
#include "render_panel/ui/tab_hw_info.h"
#include "gr_settings.h"
#include "gr_context.h"
#include "gr_render_controller.h"
#include "gr_app_messages.h"
#include "service/service_manager.h"
#include "app_colors.h"
#include "render_panel/ui/tab_server_status.h"
#include "tc_qt_widget/widgetframe/mainwindow_wrapper.h"
#include "tc_qt_widget/widgetframe/titlebar_messages.h"
#include "tc_qt_widget/tc_dialog.h"
#include "version_config.h"
#include "tc_common_new/win32/process_helper.h"
#include "tc_label.h"
#include "no_margin_layout.h"
#include "ui/user/user_login_dialog.h"
#include "ui/user/user_register_dialog.h"
#include "tc_spvr_client/spvr_user_api.h"
#include "ui/tab_cophone.h"
#include "skin/interface/skin_interface.h"
#include "user/gr_user_manager.h"
#include "tc_common_new/file.h"
#include "tc_common_new/file_util.h"
#include "tc_common_new/http_client.h"
#include "tc_common_new/dump_helper.h"
#include "tc_qt_widget/tc_dialog_util.h"
#include "tc_qt_widget/round_img_display.h"
#include "tc_qt_widget/image_cropper/image_cropper_dialog.h"
#include "render_panel/ui/user/modify_username_dialog.h"
#include "render_panel/ui/user/modify_password_dialog.h"
#include "render_panel/companion/panel_companion.h"
#include "render_panel/upgrade/upgrade_helper.h"

namespace tc
{

    std::shared_ptr<GrWorkspace> grWorkspace;

    GrWorkspace::GrWorkspace(bool run_automatically) : QMainWindow(nullptr) {
        this->run_automatically_ = run_automatically;
        settings_ = GrSettings::Instance();
        //setWindowFlags(windowFlags() | Qt::ExpandedClientAreaHint | Qt::NoTitleBarBackgroundHint);
        WidgetHelper::SetTitleBarColor(this);

        auto menu = new QMenu(this);
        sys_tray_icon_ = new QSystemTrayIcon(this);
        sys_tray_icon_->setIcon(QIcon(":/resources/tc_icon.png"));
        sys_tray_icon_->setToolTip(tr("GoDesk"));

        auto ac_show = new QAction(tcTr("id_show_panel"), this);
        auto ac_exit = new QAction(tcTr("id_exit_all_programs"), this);

        connect(ac_show, &QAction::triggered, this, [=, this](bool) {
            this->showNormal();
        });

        connect(ac_exit, &QAction::triggered, this, [=, this](bool) {
            this->ForceStopAllPrograms(false);
        });

        menu->addAction(ac_show);
        menu->addAction(ac_exit);
        sys_tray_icon_->setContextMenu(menu);
        sys_tray_icon_->show();
        connect(sys_tray_icon_, &QSystemTrayIcon::activated, this, [=, this](QSystemTrayIcon::ActivationReason reason) {
            if (QSystemTrayIcon::ActivationReason::DoubleClick == reason || QSystemTrayIcon::ActivationReason::Trigger == reason) {
                this->showNormal();
                this->raise();
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

        app_ = std::make_shared<GrApplication>(this, run_automatically);
        app_->Init();
        context_ = app_->GetContext();
        skin_ = grApp->GetSkin();

        user_mgr_ = grApp->GetUserManager();

        std::string version = "";
#if PREMIUM_VERSION
        if (skin_) {
            version = skin_->GetAppVersionMode().toStdString();
        }
        if (version == "Premium") {
            version = tcTr("id_version_premium").toStdString();
        }
        else {
            version = "Premium";
        }
#else
        version = "Freemium";
#endif
        if (skin_) {
            setWindowTitle(std::format("{}(V{} {})", skin_->GetAppName().toStdString(),
                                       skin_->GetAppVersionName().toStdString(),
                                       version).c_str());
        }
        else {
            setWindowTitle(std::format("Godesk(V{} {})", PROJECT_VERSION, version).c_str());
        }

        qApp->installNativeEventFilter(app_.get());

        // window
        auto notifier = app_->GetMessageNotifier();

        // background
        setStyleSheet(R"(QMainWindow {background-color:#FFFFFF;})");

        // root
        auto root_layout = new QHBoxLayout();
        WidgetHelper::ClearMargins(root_layout);

        // left buttons
        {
            auto layout = new QVBoxLayout();
            WidgetHelper::ClearMargins(layout);

            // placeholder to extend the width of left area
            int left_area_width = 220;
            auto extend = new QLabel(this);
            extend->setFixedSize(left_area_width, 2);
            layout->addWidget(extend);

            // logo
            {
                // logo
                auto logo_layout = new NoMarginHLayout();
                auto logo = new RoundImageDisplay("", avatar_size_, avatar_size_, avatar_size_/2);
                logo->SetBorder(2, 0x555555);
                lbl_avatar_ = logo;
                logo->setFixedSize(avatar_size_, avatar_size_);
                logo->setScaledContents(true);
                auto pixmap = WidgetHelper::RenderSvgToPixmap(":/resources/image/ic_not_login.svg", QSize(avatar_size_, avatar_size_));
                logo->UpdatePixmap(pixmap);
                logo_layout->addSpacing(20);
                logo_layout->addWidget(logo);
                logo->SetOnClickListener([=, this](QWidget* w) {
                    if (user_mgr_->IsLoggedIn()) {
                        ShowUserActions();
                    }
                    else {
                        this->ShowUserLoginDialog();
                    }
                });
                logo_layout->addSpacing(8);

                // name
                auto name_layout = new NoMarginVLayout();
                name_layout->addStretch();
                auto lbl = new TcLabel(this);
                lbl_username_ = lbl;
                lbl->setMaximumWidth(125);
                lbl->setStyleSheet("font-weight: 700; color: #333333; font-size: 15px;");

                UpdateUsername();

                lbl->SetOnClickListener([=, this](QWidget* w) {
                    if (user_mgr_->IsLoggedIn()) {
                        ShowUserActions();
                    }
                    else {
                        this->ShowUserLoginDialog();
                    }
                });
                name_layout->addWidget(lbl);

                name_layout->addSpacing(3);

                auto lbl_version = new TcLabel(this);
#if PREMIUM_VERSION
                lbl_version->setStyleSheet("font-weight: 700; color: #2979ff; font-size: 12px;");
#else
                lbl_version->setStyleSheet("font-weight: 700; color: #666666; font-size: 12px;");
#endif
                lbl_version->setText(version.c_str());
                name_layout->addWidget(lbl_version);

                name_layout->addStretch();
                logo_layout->addLayout(name_layout);
                logo_layout->addStretch();
                //

                layout->addSpacing(45);
                layout->addLayout(logo_layout);
            }

            // buttons
            auto btn_font_color = "#ffffff";
            auto btn_size = QSize(left_area_width - 30, 40);
            // remote control
            {
                auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
                btn->AddIcon(":/resources/image/ic_stream_selected.svg", ":/resources/image/ic_stream_normal.svg", 20, 20);
                btn_tab_server_ = btn;
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetTextId("id_tab_remote_control");
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    ChangeTab(TabName::kTabServer);
                });
                layout->addSpacing(30);
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            // server status
            {
                auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
                btn->AddIcon(":/resources/image/ic_statistics_selected.svg", ":/resources/image/ic_statistics_normal.svg", 20, 20);
                btn_tab_server_status_ = btn;
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetTextId("id_tab_server_status");
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    ChangeTab(TabName::kTabServerStatus);
                });
                layout->addSpacing(10);
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            if (skin_->IsGameEnabled()) {
                auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
                btn->AddIcon(":/resources/image/ic_game_selected.svg", ":/resources/image/ic_game_normal.svg", 20, 20);
                btn_tab_games_ = btn;
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetTextId("id_tab_games");
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    ChangeTab(TabName::kTabGames);
                });
                layout->addSpacing(10);
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            if (skin_->IsCoPhoneEnabled()) {
                auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
                btn->AddIcon(":/resources/image/ic_device_selected.svg", ":/resources/image/ic_device_normal.svg", 20, 20);
                btn_tab_cophone_ = btn;
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetTextId("id_co_phone");
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    ChangeTab(TabName::kTabCoPhone);
                });
                layout->addSpacing(10);
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            {
                auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
                btn->AddIcon(":/resources/image/ic_settings_security_selected.svg", ":/resources/image/ic_settings_security_normal.svg", 20, 20);
                btn_security_ = btn;
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetTextId("id_tab_security");
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    ChangeTab(TabName::kTabSecurity);
                });
                layout->addSpacing(10);
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            {
                auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
                btn->AddIcon(":/resources/image/ic_settings_outline_selected.svg", ":/resources/image/ic_settings_outline_normal.svg", 20, 20);
                btn_tab_settings_ = btn;
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetTextId("id_settings");
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    ChangeTab(TabName::kTabSettings);
                });
                layout->addSpacing(10);
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            {
                auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
                btn->AddIcon(":/resources/image/ic_hw_selected.svg", ":/resources/image/ic_hw_normal.svg", 20, 20);
                btn_tab_hw_info_ = btn;
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetTextId("id_tab_hardware");
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    ChangeTab(TabName::kTabHWInfo);
                });
                layout->addSpacing(10);
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            /// Splitter
            layout->addStretch(100);

            auto exit_btn_size = QSize(btn_size.width(), btn_size.height() - 5);

            // jump to github
            {
                QWidget* w = new QWidget(this);
                jump_to_github_widget_ = w;
                w->setObjectName("jump_github");
                w->setStyleSheet(R"(
                    #jump_github {
                        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #5578E8, stop:1 #6488E8);
                        border-radius: 5px;
                        padding: 5px;
                    }
                    #jump_github:hover {
                        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, 
                                                    stop:0 #5578E8, stop:1 #6488E8);
                    }
                    #jump_github:pressed {
                        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, 
                                                    stop:0 #5578FE8, stop:1 #6488E8);
                    }
                )");

                w->setFixedSize(btn_size.width(), btn_size.height() * 2.2);
                
                layout->addWidget(w, 0, Qt::AlignHCenter);
                layout->addSpacing(10);

                QVBoxLayout* vlayout = new QVBoxLayout(w);
                vlayout->setSpacing(10);
                vlayout->setContentsMargins(4, 4, 4, 4);

                QLabel* label = new QLabel(w);
                label->setText(tcTr("id_find_new_version"));
                label->setStyleSheet(R"(
                    QLabel {
                        font-size: 16px;
                        color: #ffffff;
                    }
                )");
                vlayout->addWidget(label, 0, Qt::AlignCenter);

                QPushButton* btn = new QPushButton(w);
                btn->setText(tcTr("id_click_to_down"));
                btn->setFixedSize(btn_size.width()-20, btn_size.height() - 6);
                btn->setStyleSheet(R"(
                    QPushButton{
                        color: white;
                        text - decoration: underline;
                        border: none;
                        padding: 8px 16px;
                        font-size: 14px;
                    }
                )");
                btn->setCursor(QCursor(Qt::PointingHandCursor));
                vlayout->addWidget(btn, 0, Qt::AlignCenter);

                connect(btn, &QPushButton::clicked, this, [=, this]() {
                    auto pc = grApp->GetCompanion();
                    if (pc) {
                        pc->JumpToGithub();
                    }
                });

                app_->GetContext()->PostTask([=, this]() {
                    auto pc = grApp->GetCompanion();
                    if (!pc) {
                        return;
                    }
                    if (pc->HasUpdateForOffSite()) {
                        app_->GetContext()->PostUITask([=, this]() {
                            w->show();
                        });
                    } else {
                        app_->GetContext()->PostUITask([=, this]() {
                            w->hide();
                        });
                    }
                });
            }
           
            // stop all
            {
                auto btn = new QPushButton(this);
                btn_exit_ = btn;
                btn->setText(tcTr("id_exit_all_programs"));
                btn->setProperty("class", "danger");
                //btn->setProperty("flat", true);
                btn->setFixedSize(exit_btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    this->ForceStopAllPrograms(false);
                });
                layout->addWidget(btn, 0, Qt::AlignHCenter);
                layout->addSpacing(5);

                btn->setHidden(!settings_->IsDevelopMode());
            }

            // uninstall all
            {
                auto btn = new QPushButton(this);
                btn_uninstall_ = btn;
                btn->setText(tcTr("id_uninstall_all_programs"));
                btn->setProperty("class", "danger");
                //btn->setProperty("flat", true);
                btn->setFixedSize(exit_btn_size);
                QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                    this->ForceStopAllPrograms(true);
                });
                layout->addWidget(btn, 0, Qt::AlignHCenter);
                layout->addSpacing(5);

                btn->setHidden(!settings_->IsDevelopMode());
            }

            {
                auto lbl = new QLabel(this);
                lbl->setFixedSize(190, 40);
                auto p = skin_->GetLargeIconTextLogo();
                lbl->setPixmap(p);
                layout->addSpacing(8);
                layout->addWidget(lbl, 0, Qt::AlignHCenter);
                layout->addSpacing(8);
            }

            root_layout->addLayout(layout);

            QTimer::singleShot(2000, this, [this]() {
                this->InitUpdate();
            });
        }

        // right panels
        {
            // tabs
            tabs_.insert({TabName::kTabServer, new TabServer(app_, this)});
            tabs_.insert({TabName::kTabServerStatus, new TabServerStatus(app_, this)});
            if (skin_->IsGameEnabled()) {
                tabs_.insert({TabName::kTabGames, new TabGame(app_, this)});
            }
            if (skin_->IsCoPhoneEnabled()) {
                tabs_.insert({TabName::kTabCoPhone, new TabCoPhone(app_, this)});
            }
            tabs_.insert({TabName::kTabSettings, new TabSettings(app_, this)});
            tabs_.insert({TabName::kTabSecurity, new TabSecurityInternals(app_, this)});
            //tabs_.insert({TabName::kTabProfile, new TabProfile(app_, this)});
            tabs_.insert({TabName::kTabHWInfo, new TabHWInfo(app_, this)});

            tabs_[TabName::kTabServer]->SetAttach(btn_tab_server_);
            tabs_[TabName::kTabServerStatus]->SetAttach(btn_tab_server_status_);
            if (skin_->IsGameEnabled()) {
                tabs_[TabName::kTabGames]->SetAttach(btn_tab_games_);
            }
            if (skin_->IsCoPhoneEnabled()) {
                tabs_[TabName::kTabCoPhone]->SetAttach(btn_tab_cophone_);
            }
            tabs_[TabName::kTabSettings]->SetAttach(btn_tab_settings_);
            tabs_[TabName::kTabSecurity]->SetAttach(btn_security_);
            //tabs_[TabName::kTabProfile]->SetAttach(btn_tab_profile_);
            tabs_[TabName::kTabHWInfo]->SetAttach(btn_tab_hw_info_);

            auto layout = new QVBoxLayout();
            WidgetHelper::ClearMargins(root_layout);
            auto stack_widget = new QStackedWidget(this);
            stack_widget->addWidget(tabs_[TabName::kTabServer]);
            stack_widget->addWidget(tabs_[TabName::kTabServerStatus]);
            if (skin_->IsGameEnabled()) {
                stack_widget->addWidget(tabs_[TabName::kTabGames]);
            }
            if (skin_->IsCoPhoneEnabled()) {
                stack_widget->addWidget(tabs_[TabName::kTabCoPhone]);
            }
            stack_widget->addWidget(tabs_[TabName::kTabSettings]);
            stack_widget->addWidget(tabs_[TabName::kTabSecurity]);
            //stack_widget->addWidget(tabs_[TabName::kTabProfile]);
            stack_widget->addWidget(tabs_[TabName::kTabHWInfo]);
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

        // last works
        app_->RequestNewClientId(false);

        //
        if (user_mgr_->IsLoggedIn()) {
            this->Login();
        }

        InitListeners();
    }

    void GrWorkspace::Init() {
        grWorkspace = shared_from_this();
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
        if (upgrade_helper_widget_) {
            upgrade_helper_widget_->done(QDialog::Rejected);
            upgrade_helper_widget_->close();
            upgrade_helper_widget_ = nullptr;
        }
        event->ignore();
        TcDialog dialog(tcTr("id_hide"), tcTr("id_hide_gammaray_msg"), this);
        if (kDoneOk == dialog.exec()) {
            this->hide();
        }
    }

    void GrWorkspace::resizeEvent(QResizeEvent *event) {
        QMainWindow::resizeEvent(event);
    }

    void GrWorkspace::InitListeners() {
        msg_listener_ = app_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgTitleBarSettingsClicked>([=, this](const MsgTitleBarSettingsClicked& msg) {
            app_->GetContext()->PostUITask([=, this]() {
                ChangeTab(TabName::kTabSettings);
            });
        });

        msg_listener_->Listen<MsgTitleBarAvatarClicked>([=, this](const MsgTitleBarAvatarClicked& msg) {
            app_->GetContext()->PostUITask([=, this]() {

            });
        });

        // develop mode update
        msg_listener_->Listen<MsgDevelopModeUpdated>([=, this](const MsgDevelopModeUpdated& msg) {
            app_->GetContext()->PostUITask([=, this]() {
                btn_exit_->setHidden(!msg.enabled_);
                btn_uninstall_->setHidden(!msg.enabled_);
            });
        });

        // force stop all programs
        msg_listener_->Listen<MsgForceStopAllPrograms>([=, this](const MsgForceStopAllPrograms& msg) {
            app_->GetContext()->PostUITask([=, this]() {
                this->ForceStopAllPrograms(msg.uninstall_service_);
            });
        });

        // clear data
        msg_listener_->Listen<MsgForceClearProgramData>([=, this](const MsgForceClearProgramData& msg) {
            context_->PostUITask([=, this]() {
                this->ClearUserInfo();
            });
        });

        // check update
        msg_listener_->Listen<MsgCheckUpdate>([=, this](const MsgCheckUpdate& msg) {
            app_->GetContext()->PostUITask([=, this]() {
                this->CheckAppUpdate(true);
            });
        });

        // update
        msg_listener_->Listen<MsgGrTimer10H>([=, this](const MsgGrTimer10H& msg) {
            {
                app_->GetContext()->PostUITask([=, this]() {
                    this->CheckAppUpdate(false);
                });

                app_->GetContext()->PostTask([=]() {
                    tc::ClearOldDumps();
                });
            }

            {
                // jump to github
                auto pc = grApp->GetCompanion();
                if (!pc) {
                    return;
                }
                if (pc->HasUpdateForOffSite()) {
                    app_->GetContext()->PostUITask([=, this]() {
                        jump_to_github_widget_->show();
                    });
                }
                else {
                    app_->GetContext()->PostUITask([=, this]() {
                        jump_to_github_widget_->hide();
                    });
                }
            }
        });
    }

    void GrWorkspace::ForceStopAllPrograms(bool uninstall_service) {
        TcDialog dialog(tcTr("id_exit"), uninstall_service ? tcTr("id_uninstall_gammaray_msg") : tcTr("id_exit_gammaray_msg"), this);
        if (dialog.exec() == kDoneOk) {
            auto srv_mgr = this->app_->GetContext()->GetServiceManager();
            srv_mgr->Remove(uninstall_service);

            app_->GetContext()->PostDelayTask([=, this]() {
                auto processes = tc::ProcessHelper::GetProcessList(false);
                for (auto& process : processes) {
                    if (process->exe_full_path_.find(kGammaRayGuardName) != std::string::npos) {
                        LOGI("Kill exe: {}", process->exe_full_path_);
                        tc::ProcessHelper::CloseProcess(process->pid_);
                        break;
                    }
                }
                for (auto& process : processes) {
                    if (process->exe_full_path_.find(kGammaRayClientInner) != std::string::npos) {
                        LOGI("Kill exe: {}", process->exe_full_path_);
                        tc::ProcessHelper::CloseProcess(process->pid_);
                        break;
                    }
                }
                for (auto& process : processes) {
                    if (process->exe_full_path_.find(kGammaRayRenderName) != std::string::npos) {
                        LOGI("Kill exe: {}", process->exe_full_path_);
                        tc::ProcessHelper::CloseProcess(process->pid_);
                        break;
                    }
                }
                for (auto& process : processes) {
                    if (process->exe_full_path_.find(kGammaRaySysInfo) != std::string::npos) {
                        LOGI("Kill exe: {}", process->exe_full_path_);
                        tc::ProcessHelper::CloseProcess(process->pid_);
                    }
                }
                for (auto& process : processes) {
                    if (process->exe_full_path_.find(kGammaRayName) != std::string::npos) {
                        LOGI("Kill exe: {}", process->exe_full_path_);
                        tc::ProcessHelper::CloseProcess(process->pid_);
                    }
                }
            }, 1000);
        }
    }

    void GrWorkspace::ShowUserRegisterDialog() {
        UserRegisterDialog dialog(app_->GetContext());
        auto r = dialog.exec();
        if (r == 0) {
            UpdateUserInfo();
        }
    }

    void GrWorkspace::Login() {
        context_->PostTask([=, this]() {
            auto username = user_mgr_->GetUsername();
            auto password = user_mgr_->GetPassword();
            if (username.empty() || password.empty()) {
                return;
            }
            user_mgr_->Login(username, password, false);

            this->LoadAvatar();
        });
    }

    void GrWorkspace::ShowUserLoginDialog() {
        UserLoginDialog dialog(app_->GetContext());
        auto r = dialog.exec();
        if (r == -1) {
            //
            ShowUserRegisterDialog();
        }
        else if (r == 0) {
            UpdateUserInfo();
        }
    }

    void GrWorkspace::ShowSelectAvatarDialog() {
        auto desktop_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        auto image_path = TcDialogUtil::SelectImage(tcTr("id_select_image"), desktop_path, nullptr);
        if (image_path.isEmpty()) {
            return;
        }
        QPixmap image = ImageCropperDialog::getCroppedImage(image_path, 600, 400, CropperShape::CIRCLE);
        if (image.isNull()) {
            return;
        }

        auto avatar_path = settings_->GetGrDataCachePath() + "/" + grApp->GetUserManager()->GetUserId() + "_avatar.jpg";
        image.save(avatar_path.c_str());

        if (!File::Exists(avatar_path)) {
            LOGE("Crop image failed, file not exists: {}", avatar_path);
            return;
        }

        auto size = File::Size(avatar_path);
        if (size < 0 || size >= 10 * 1024 * 1024) {
            LOGE("Image size invalid: {}", size);
            return;
        }

        if (user_mgr_->UpdateAvatar(avatar_path)) {
            LoadAvatar();
        }
    }

    void GrWorkspace::UpdateUserInfo() {
        UpdateUsername();
        LoadAvatar();
    }

    void GrWorkspace::ClearUserInfo() {
        lbl_username_->SetTextId("id_guest");
    }

    void GrWorkspace::UpdateUsername() {
        if (user_mgr_->IsLoggedIn()) {
            lbl_username_->SetTextId("");
            lbl_username_->setText(user_mgr_->GetUsername().c_str());
        }
        else {
            lbl_username_->SetTextId("id_guest");
        }
    }

    void GrWorkspace::ShowUserActions() {
        auto menu = new QMenu();
        std::vector<QString> actions = {
            tcTr("id_edit_username"),
            tcTr("id_edit_password"),
            tcTr("id_edit_avatar"),
            "",
            tcTr("id_user_center"),
            "",
            tcTr("id_exit_login"),
        };
        for (int i = 0; i < actions.size(); i++) {
            const QString& action_name = actions.at(i);
            if (action_name.isEmpty()) {
                menu->addSeparator();
                continue;
            }

            auto action = new QAction(action_name, menu);
            menu->addAction(action);
            connect(action, &QAction::triggered, this, [=, this]() {
                ProcessUserAction(i);
            });
        }
        menu->exec(QCursor::pos());
        delete menu;
    }

    void GrWorkspace::ProcessUserAction(int index) {
        // modify username
        if (index == 0) {
            ModifyUsernameDialog dialog(context_);
            if (dialog.exec() == kDoneOk) {
                this->UpdateUsername();
            }
        }
        else if (index == 1) {
            // modify password
            ModifyPasswordDialog dialog(context_);
            if (dialog.exec() == kDoneOk) {

            }
        }
        else if (index == 2) {
            // update avatar
            ShowSelectAvatarDialog();
        }
        else if (index == 4) {
            // user center
        }
        else if (index == 6) {
            // exit
            TcDialog dialog(tcTr("id_exit_login"), tcTr("id_exit_login_msg"));
            if (dialog.exec() == kDoneOk) {
                // logout
                user_mgr_->Logout();

                // clear avatar
                ClearAvatar();

                // clear database
                user_mgr_->Clear();

                // clear ui
                ClearUserInfo();
                // avatar

                // send a logged in message
                context_->SendAppMessage(MsgUserLoggedOut {});
            }
        }
    }

    void GrWorkspace::LoadAvatar() {
        context_->PostTask([=, this]() {
            auto avatar_path = user_mgr_->GetAvatarPath();
            if (avatar_path.starts_with("./")) {
                avatar_path = avatar_path.substr(1);
            }
            auto avatar_url_path = std::format("https://{}:{}{}?appkey={}", settings_->GetSpvrServerHost(), settings_->GetSpvrServerPort(), avatar_path, grApp->GetAppkey());
            auto target_avatar_path = settings_->GetGrDataCachePath() + "/" + user_mgr_->GetUserId() + + "." + FileUtil::GetFileSuffix(avatar_path);
            LOGI("Cached avatar path: {}", target_avatar_path);
            if (File::Exists(target_avatar_path)) {
                LOGI("Load local avatar first");
                context_->PostUITask([=, this]() {
                    this->SetAvatar(target_avatar_path);
                });
            }

            auto target_avatar_cache_path = settings_->GetGrDataCachePath() + "/" + user_mgr_->GetUserId() + + "_cache." + FileUtil::GetFileSuffix(avatar_path);
            auto file = File::OpenForWriteB(target_avatar_cache_path);
            auto r = HttpClient::Download(avatar_url_path, [=, this](const std::string& d) {
                file->Append(d);
            });
            if (r.status == 200) {
                LOGI("Load avatar from server and refresh it!");
                file->Close();
                File::Delete(target_avatar_path);
                FileUtil::ReName(target_avatar_cache_path, target_avatar_path);
                File::Delete(target_avatar_cache_path);
                context_->PostUITask([=, this]() {
                    this->SetAvatar(target_avatar_path);
                });
            }
            else {
                file->Close();
            }
        });
    }

    void GrWorkspace::SetAvatar(const std::string& filepath) {
        auto pixmap = QPixmap::fromImage(QImage(filepath.c_str()));
        if (pixmap.isNull()) {
            return;
        }
        pixmap = pixmap.scaled(avatar_size_, avatar_size_, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        lbl_avatar_->UpdatePixmap(pixmap);
    }

    void GrWorkspace::ClearAvatar() {
        auto pixmap = WidgetHelper::RenderSvgToPixmap(":/resources/image/ic_not_login.svg", QSize(avatar_size_, avatar_size_));
        lbl_avatar_->UpdatePixmap(pixmap);
    }


    void GrWorkspace::InitUpdate() {
        QObject::connect(UpdateManager::GetInstance(), &UpdateManager::SigFindUpdate, [this](const QVariantMap& data) {
            this->showNormal();
            
            if (upgrade_helper_widget_) {
                upgrade_helper_widget_->close();
                upgrade_helper_widget_ = nullptr;
            }
            
            upgrade_helper_widget_ = QPointer<UpgradeHelperWidget>(new UpgradeHelperWidget());
            upgrade_helper_widget_->SetRemoteVersion(data["version"].toString());
            upgrade_helper_widget_->SetRemoteUpdateDesc(data["desc"].toString());
            upgrade_helper_widget_->SetForced(data["forced"].toBool());
            upgrade_helper_widget_->raise();
            upgrade_helper_widget_->exec();
            if (upgrade_helper_widget_->exit_app_) {
                this->close();
            }
        });

        QObject::connect(UpdateManager::GetInstance(), &UpdateManager::SigUpdateHint, [this](QString info) {
            this->showNormal();
            TcDialog dialog(tcTr("id_tips"), info, this);
            dialog.exec();
        });

        UpdateManager::GetInstance()->CheckUpdate(true, false);
    }

    void GrWorkspace::CheckAppUpdate(bool from_user_clicked) {
        tc::UpdateManager::GetInstance()->CheckUpdate(true, from_user_clicked);
    }
}
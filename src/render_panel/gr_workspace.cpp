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
        sys_tray_icon_->setToolTip(tr("GammaRay"));

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
        skin_ = grApp->GetSkin();

        auto version = "";
#if PREMIUM_VERSION
        version = "Premium";
#else
        version = "Freemium";
#endif
        if (skin_) {
            setWindowTitle(std::format("{}(V{} {})", skin_->GetAppName().toStdString(),
                                       skin_->GetAppVersionName().toStdString(),
                                       skin_->GetAppVersionMode().toStdString()).c_str());
        }
        else {
            setWindowTitle(std::format("GammaRay(V{} {})", PROJECT_VERSION, version).c_str());
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
                auto logo = new TcLabel(this);
                int logo_size = 50;
                logo->setFixedSize(logo_size, logo_size);
                logo->setScaledContents(true);
                auto pixmap = WidgetHelper::RenderSvgToPixmap(":/resources/image/ic_not_login.svg", QSize(logo_size, logo_size));
                logo->setPixmap(pixmap);
                logo_layout->addSpacing(20);
                logo_layout->addWidget(logo);
                logo->SetOnClickListener([=, this](QWidget* w) {
                    this->ShowUserRegisterDialog();
                });
                logo_layout->addSpacing(8);

                // name
                auto name_layout = new NoMarginVLayout();
                name_layout->addStretch();
                auto lbl = new TcLabel(this);
                lbl->setMaximumWidth(125);
                lbl->setStyleSheet("font-weight: 700; color: #333333; font-size: 15px;");
                lbl->SetTextId("id_guest");
                lbl->SetOnClickListener([=, this](QWidget* w) {
                    this->ShowUserRegisterDialog();
                });
                name_layout->addWidget(lbl);

                name_layout->addSpacing(3);

                auto lbl_version = new TcLabel(this);
#if PREMIUM_VERSION
                lbl_version->setStyleSheet("font-weight: 700; color: #2979ff; font-size: 12px;");
#else
                lbl_version->setStyleSheet("font-weight: 700; color: #666666; font-size: 12px;");
#endif
                lbl_version->setText(version);
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

            {
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
//                lbl->setStyleSheet(R"(
//                    border: none;
//                    border-image: url(:/resources/tc_logo_text_trans_bg.png);
//                    background-repeat: no-repeat;
//                    background-position: center;
//                )");
                auto p = skin_->GetLargeIconTextLogo();
                lbl->setPixmap(p);
                layout->addSpacing(8);
                layout->addWidget(lbl, 0, Qt::AlignHCenter);
                layout->addSpacing(8);
            }

            root_layout->addLayout(layout);
        }

        // right panels
        {
            // tabs
            tabs_.insert({TabName::kTabServer, new TabServer(app_, this)});
            tabs_.insert({TabName::kTabServerStatus, new TabServerStatus(app_, this)});
            if (skin_->IsGameEnabled()) {
                tabs_.insert({TabName::kTabGames, new TabGame(app_, this)});
            }
            tabs_.insert({TabName::kTabCoPhone, new TabCoPhone(app_, this)});
            tabs_.insert({TabName::kTabSettings, new TabSettings(app_, this)});
            tabs_.insert({TabName::kTabSecurity, new TabSecurityInternals(app_, this)});
            //tabs_.insert({TabName::kTabProfile, new TabProfile(app_, this)});
            tabs_.insert({TabName::kTabHWInfo, new TabHWInfo(app_, this)});

            tabs_[TabName::kTabServer]->SetAttach(btn_tab_server_);
            tabs_[TabName::kTabServerStatus]->SetAttach(btn_tab_server_status_);
            if (skin_->IsGameEnabled()) {
                tabs_[TabName::kTabGames]->SetAttach(btn_tab_games_);
            }
            tabs_[TabName::kTabCoPhone]->SetAttach(btn_tab_cophone_);
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
            stack_widget->addWidget(tabs_[TabName::kTabCoPhone]);
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
//        UserLoginDialog login_dialog(app_->GetContext());
//        login_dialog.exec();

        UserRegisterDialog dialog(app_->GetContext());
        dialog.exec();

        auto username = dialog.GetInputUsername();
        auto password = dialog.GetInputPassword();
        auto opt_user = spvr::SpvrUserApi::Register(settings_->GetSpvrServerHost(), settings_->GetSpvrServerPort(), grApp->GetAppkey(), username, password);
        if (!opt_user.has_value()) {
            LOGI("Register user failed!");
            return;
        }
        auto user = opt_user.value();
        LOGI("Register success: {}, {}", user->username_, user->uid_);

    }

}
//
// Created by RGAA on 4/02/2025.
//

#include "tab_server_status.h"

#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <utility>
#include <QComboBox>
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "tc_common_new/qrcode/qr_generator.h"
#include "tc_qt_widget/widget_helper.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_qt_widget/round_img_display.h"
#include "tc_qt_widget/tc_label.h"
#include "tc_qt_widget/tc_pushbutton.h"
#include "rn_app.h"
#include "rn_empty.h"
#include "tc_common_new/message_notifier.h"
#include "render_panel/gr_app_messages.h"
#include "tc_common_new/log.h"
#include "qt_circle.h"
#include "qt_vertical.h"
#include "render_panel/gr_statistics.h"
#include "render_panel/gr_application.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "render_panel/gr_render_controller.h"
#include "service/service_manager.h"
#include "tc_common_new/uid_spacer.h"
#include "render_panel/gr_run_game_manager.h"
#include "render_panel/database/db_game.h"
#include "tc_qt_widget/tc_dialog.h"

namespace tc
{

    static QString GetItemIconStyleSheet(const QString &url) {
        QString style = R"(background-image: url(%1);
                        background-repeat: no-repeat;
                        background-position: center;
                    )";
        return style.arg(url);
    }

    TabServerStatus::TabServerStatus(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        auto content_root = new NoMarginHLayout();

        // LEFT
        auto label_width = 195;
        int margin_left = 20;
        {
            auto layout = new NoMarginVLayout();
            // title margin
            layout->addSpacing(kTabContentMarginTop);

            content_root->addLayout(layout);
            // Server Status
            {
                auto item_layout = new NoMarginHLayout();
                auto title = new TcLabel(this);
                title->SetTextId("id_server_status");
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 22px; font-weight:700;)");
                item_layout->addSpacing(margin_left + 9);
                item_layout->addWidget(title);
                item_layout->addStretch();
                layout->addLayout(item_layout);
                layout->addSpacing(8);
            }

#if PREMIUM_VERSION
            // driver status
            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(38, 38);
                icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_game_controller.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_joystick_driver");
                label->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(label);

                auto status = new TcLabel(this);
                lbl_vigem_state_ = status;
                status->setAlignment(Qt::AlignCenter);
                status->setFixedSize(80, 26);
                status->setText("OK");
                item_layout->addWidget(status);

                auto btn_install = new TcPushButton(this);
                btn_install->setFixedSize(80, 28);
                btn_install->SetTextId("id_install");
                item_layout->addSpacing(40);
                item_layout->addWidget(btn_install);

                auto btn_remove = new TcPushButton(this);
                btn_remove->hide();
                btn_remove->setFixedSize(80, 28);
                btn_remove->SetTextId("id_remove");
                item_layout->addSpacing(5);
                item_layout->addWidget(btn_remove);
                item_layout->addStretch();

                connect(btn_install, &TcPushButton::clicked, this, [=, this]() {
                    TcDialog dialog(tcTr("id_install_joystick_driver"), tcTr("id_install_joystick_driver_msg"), this);
                    if (dialog.exec() == kDoneOk) {
                        context_->SendAppMessage(MsgInstallViGEm{});
                    }
                });

                layout->addLayout(item_layout);
            }
#endif

            // server status
            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(38, 38);
                icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_renderer.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_renderer_status");
                label->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(label);

                auto status = new TcLabel(this);
                lbl_renderer_state_ = status;
                status->setAlignment(Qt::AlignCenter);
                status->setFixedSize(80, 26);
                status->setText("OK");
                item_layout->addWidget(status);

                auto btn_restart = new TcPushButton(this);
                btn_restart->setFixedSize(80, 28);
                btn_restart->SetTextId("id_restart");
                item_layout->addSpacing(40);
                item_layout->addWidget(btn_restart);

                auto btn_remove = new TcPushButton(this);
                btn_remove->hide();
                btn_remove->setFixedSize(80, 28);
                btn_remove->setText(tr("REMOVE"));
                item_layout->addSpacing(5);
                item_layout->addWidget(btn_remove);
                item_layout->addStretch();

                connect(btn_restart, &TcPushButton::clicked, this, [=, this]() {
                    TcDialog dialog(tcTr("id_restart_renderer"), tcTr("id_restart_renderer_msg"), this);
                    if (dialog.exec() == kDoneOk) {
                        this->context_->PostTask([=, this]() {
                            RpRestartServer();
                        });
                    }
                });

                layout->addLayout(item_layout);
            }

            // service status
            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(38, 38);
                icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_service.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_service_status");
                label->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(label);

                auto status = new TcLabel(this);
                lbl_service_state_ = status;
                status->setAlignment(Qt::AlignCenter);
                status->setFixedSize(80, 26);
                status->setText("OK");
                item_layout->addWidget(status);

                auto btn_install = new TcPushButton(this);
                btn_install->setFixedSize(80, 28);
                btn_install->SetTextId("id_install");
                item_layout->addSpacing(40);
                item_layout->addWidget(btn_install);
                item_layout->addStretch();

                connect(btn_install, &TcPushButton::clicked, this, [=, this]() {
                    TcDialog dialog(tcTr("id_install_service"), tcTr("id_install_service_msg"), this);
                    if (dialog.exec() == kDoneOk) {
                        this->context_->GetServiceManager()->Install();
                    }
                });

                layout->addLayout(item_layout);
            }

            // IPs
            auto ips = context_->GetIps();
            for (auto& item : ips) {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(38, 38);
                icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_network.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_host_address");
                label->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                value->setFixedSize(120, 40);
                value->setText(item.ip_addr_.c_str());
                value->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(value);

                auto nt_type = new TcLabel(this);
                nt_type->setFixedSize(80, 40);
                nt_type->setText(item.nt_type_ == IPNetworkType::kWired ? "WIRE" : "WIRELESS");
                nt_type->setStyleSheet("font-size: 14px;");
                item_layout->addSpacing(10);
                item_layout->addWidget(nt_type);

                item_layout->addStretch();
                layout->addLayout(item_layout);
            }

            // http server port
            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(38, 38);
                icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_port.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_panel_tcp_port");
                label->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                value->setFixedSize(120, 40);
                value->setText(std::to_string(settings_->GetPanelServerPort()).c_str());
                value->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(value);
                item_layout->addStretch();
                layout->addLayout(item_layout);
            }

            // GammaRayRender port
            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(38, 38);
                icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_port.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_renderer_tcp_port");
                label->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                value->setFixedSize(120, 40);
                value->setText(std::to_string(settings_->GetRenderServerPort()).c_str());
                value->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(value);
                item_layout->addStretch();
                layout->addLayout(item_layout);
            }

            // GammaRayRender port
            if (0) {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(38, 38);
                icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_port.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->setText(tr("Renderer UDP Port"));
                label->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                value->setFixedSize(120, 40);
                value->setText(std::to_string(settings_->udp_listen_port_).c_str());
                value->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(value);
                item_layout->addStretch();
                layout->addLayout(item_layout);
            }

            {
                auto item_layout = new NoMarginHLayout();
                item_layout->addSpacing(margin_left);
                auto icon = new TcLabel(this);
                icon->setFixedSize(38, 38);
                icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_spectrum.svg"));
                item_layout->addWidget(icon);

                auto label = new TcLabel(this);
                label->setFixedSize(label_width, 40);
                label->SetTextId("id_audio_spectrum");
                label->setStyleSheet("font-size: 14px;");
                item_layout->addWidget(label);

                auto value = new TcLabel(this);
                value->setFixedSize(label_width, 40);
                value->setStyleSheet("font-size: 14px;");
                lbl_audio_format_ = value;
                item_layout->addWidget(value);
                item_layout->addStretch();
                layout->addLayout(item_layout);

                auto sc_layout = new NoMarginHLayout();
                sc_layout->addSpacing(margin_left);
                //spectrum_circle_ = new QtCircle(this);
                //spectrum_circle_->setFixedSize(400, 140);
                //sc_layout->addWidget(spectrum_circle_);

                spectrum_vertical_ = new QtVertical(this);
                spectrum_vertical_->setFixedSize(400, 140);
                sc_layout->addSpacing(10);
                sc_layout->addWidget(spectrum_vertical_);
                sc_layout->addStretch();
                layout->addSpacing(15);
                layout->addLayout(sc_layout);
            }

            // Running apps
            if (0) {
                auto label_size = QSize(220, 35);
                auto wrap_layout = new NoMarginHLayout();
                auto running_layout = new NoMarginVLayout();
                wrap_layout->addSpacing(margin_left + 9);
                wrap_layout->addLayout(running_layout);
                {
                    auto label = new TcLabel(this);
                    label->setFixedSize(label_size);
                    label->setText("Running Apps");
                    label->setStyleSheet(R"(font-size: 22px; font-weight:700;)");
                    running_layout->addWidget(label);
                }
                {
                    auto item_layout = new NoMarginHLayout();
                    auto label = new TcLabel(this);
                    lbl_running_games_ = label;
                    label->setFixedSize(label_size);
                    label->setText("");
                    label->setStyleSheet("font-size: 14px;");
                    item_layout->addWidget(label);
                    item_layout->addStretch();
                    running_layout->addSpacing(10);
                    running_layout->addLayout(item_layout);
                }
                running_layout->addStretch();

                layout->addSpacing(20);
                layout->addLayout(wrap_layout);
            }

            layout->addStretch();
        }

        // RIGHT
        // right part
        // Server Status
        {
            auto layout = new NoMarginVLayout();
            // title margin
            layout->addSpacing(kTabContentMarginTop);

            auto item_layout = new NoMarginHLayout();
            auto title = new TcLabel(this);
            title->SetTextId("id_statistics");
            title->setAlignment(Qt::AlignLeft);
            title->setStyleSheet(R"(font-size: 22px; font-weight:700;)");
            item_layout->addSpacing(30);
            item_layout->addWidget(title);
            item_layout->addStretch();
            layout->addLayout(item_layout);
            layout->addSpacing(8);

            rn_stack_ = new QStackedWidget(this);
            rn_app_ = new RnApp(app_, this);
            rn_stack_->addWidget(rn_app_);
            layout->addWidget(rn_stack_);

            content_root->addLayout(layout);
        }

        //content_root->addStretch();
        setLayout(content_root);

        rn_stack_->setCurrentIndex(0);

        // messages
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgViGEmState>([=, this](const MsgViGEmState& state) {
            context_->PostUITask([=, this]() {
                this->RefreshVigemState(state.ok_);
            });
        });

        msg_listener_->Listen<MsgServerAlive>([=, this](const MsgServerAlive& state) {
            context_->PostUITask([=, this]() {
                this->RefreshServerState(state.alive_);
            });
        });

        msg_listener_->Listen<MsgServiceAlive>([=, this](const MsgServiceAlive& state) {
            context_->PostUITask([=, this]() {
                this->RefreshServiceState(state.alive_);
            });
        });

        msg_listener_->Listen<MsgGrTimer100>([=, this](const auto& m) {
            context_->PostUITask([=, this]() {
                this->RefreshUIEverySecond();
            });
        });

        msg_listener_->Listen<AppMsgRestartServer>([=, this](const AppMsgRestartServer& msg) {
            context_->PostTask([=, this]() {
                RpRestartServer();
            });
        });

       msg_listener_->Listen<MsgRunningGameIds>([=, this](const MsgRunningGameIds& msg) {
           if (!lbl_running_games_) {
               return;
           }
            this->context_->PostUITask([=, this]() {
                auto rgm = this->context_->GetRunGameManager();
                auto running_games = rgm->GetRunningGames();
                std::string running_games_name;
                for (const auto& rg : running_games) {
                    running_games_name = running_games_name
                            .append(std::to_string(rg->game_->game_id_))
                            .append(" - ")
                            .append(rg->game_->game_name_).append("\n");
                }
                if (running_games_name.empty()) {
                    running_games_name = "None";
                }
                lbl_running_games_->setText(running_games_name.c_str());
            });
        });
    }

    TabServerStatus::~TabServerStatus() {

    }

    void TabServerStatus::OnTabShow() {

    }

    void TabServerStatus::OnTabHide() {

    }

    void TabServerStatus::RpRestartServer() {
        auto srv_mgr = this->context_->GetRenderController();
        //srv_mgr->StopServer();
        srv_mgr->ReStart();
        this->context_->SendAppMessage(MsgServerAlive {
            .alive_ = false,
        });
    }

    void TabServerStatus::RefreshVigemState(bool ok) {
        if (lbl_vigem_state_) {
            RefreshIndicatorState(lbl_vigem_state_, ok);
        }
    }

    void TabServerStatus::RefreshServerState(bool ok) {
        RefreshIndicatorState(lbl_renderer_state_, ok);
    }

    void TabServerStatus::RefreshServiceState(bool ok) {
        RefreshIndicatorState(lbl_service_state_, ok);
    }

    void TabServerStatus::RefreshIndicatorState(TcLabel* indicator, bool ok) {
        if (ok) {
            indicator->setStyleSheet("font-size: 13px; font-weight: bold; color:#ffffff; background:#00cc00; border-radius:13px");
            indicator->setText("OK");
        } else {
            indicator->setStyleSheet("font-size: 13px; font-weight: bold; color:#ffffff; background:#cc0000; border-radius:13px");
            indicator->setText("ERROR");
        }
    }

    void TabServerStatus::RefreshUIEverySecond() {
        if (!this->lbl_audio_format_) {
            return;
        }
        this->lbl_audio_format_->setText(
                std::format("Format: {}/{}/{}", statistics_->audio_samples_.load(), statistics_->audio_channels_.load(), statistics_->audio_bits_.load()).c_str());
    }


}
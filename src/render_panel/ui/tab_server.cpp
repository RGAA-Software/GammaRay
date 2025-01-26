//
// Created by RGAA on 2024/4/9.
//

#include "tab_server.h"

#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <utility>
#include <QPushButton>
#include <QComboBox>
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "tc_common_new/qrcode/qr_generator.h"
#include "tc_qt_widget/layout_helper.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_qt_widget/round_img_display.h"
#include "rn_app.h"
#include "rn_empty.h"
#include "tc_common_new/message_notifier.h"
#include "render_panel/app_messages.h"
#include "tc_common_new/log.h"
#include "qt_circle.h"
#include "render_panel/gr_statistics.h"
#include "render_panel/gr_application.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "render_panel/gr_render_controller.h"
#include "service/service_manager.h"
#include "tc_common_new/uid_spacer.h"

namespace tc
{

    TabServer::TabServer(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        statistics_ = GrStatistics::Instance();
        auto broadcast_msg = context_->MakeBroadcastMessage();
        qr_pixmap_ = QrGenerator::GenQRPixmap(broadcast_msg.c_str(), 200);
        // root layout
        auto root_layout = new QVBoxLayout();
        LayoutHelper::ClearMargins(root_layout);

        // content layout
        auto content_layout = new QHBoxLayout();
        LayoutHelper::ClearMargins(content_layout);

        auto item_width = 230;

        // left part
        {
            auto left_root = new NoMarginVLayout();

            // This Device
            {
                auto title = new QLabel(this);
                title->setFixedWidth(item_width);
                title->setText(tr("THIS DEVICE"));
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 27px; font-weight:700; font-family: ScreenMatrix;)"); //; padding-left: 10px
                left_root->addSpacing(18);
                left_root->addWidget(title, 0, Qt::AlignLeft);
            }

            auto machine_code_qr_layout = new NoMarginHLayout();
            left_root->addSpacing(18);
            left_root->addLayout(machine_code_qr_layout);
            content_layout->addSpacing(15);
            content_layout->addLayout(left_root);
            content_layout->addSpacing(20);

            // machine code
            {
                auto layout = new NoMarginVLayout();
                layout->addSpacing(10);

                // Machine Code //
                {
                    auto title = new QLabel(this);
                    title->setFixedWidth(item_width);
                    title->setText(tr("Machine Code"));
                    title->setAlignment(Qt::AlignLeft);
                    title->setStyleSheet(R"(font-size: 12px; font-weight:500;)");
                    //layout->addSpacing(2);
                    layout->addWidget(title, 0, Qt::AlignLeft);

                    auto msg = new QLabel(this);
                    msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    auto uid = QString::fromStdString(tc::SpaceId(context_->GetSysUniqueId()));
                    msg->setText(uid);
                    //msg->setStyleSheet(R"(font-size: 30px; font-family: ScreenMatrix;)");
                    msg->setStyleSheet(R"(font-size: 25px; font-weight: 700; color: #2979ff;)");
                    layout->addSpacing(5);
                    layout->addWidget(msg, 0, Qt::AlignLeft);
                    machine_code_qr_layout->addLayout(layout);
                }

                // Temporary Password
                {
                    layout->addSpacing(18);

                    auto title = new QLabel(this);
                    title->setFixedWidth(230);
                    title->setText(tr("Temporary Password"));
                    title->setAlignment(Qt::AlignLeft);
                    title->setStyleSheet(R"(font-size: 12px; font-weight:500;)");
                    //layout->addSpacing(2);
                    layout->addWidget(title, 0, Qt::AlignLeft);

                    auto msg = new QLabel(this);
                    msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    msg->setText("3FdsArg4");
                    //msg->setStyleSheet(R"(font-size: 30px; font-family: ScreenMatrix;)");
                    msg->setStyleSheet(R"(font-size: 25px; font-weight: 700; color: #2979ff;)");
                    layout->addSpacing(5);
                    layout->addWidget(msg, 0, Qt::AlignLeft);
                    layout->addStretch();
                    machine_code_qr_layout->addLayout(layout);
                }
            }

            {
                auto layout = new NoMarginVLayout();
                layout->addSpacing(10);
                //auto title = new QLabel(this);
                //title->setFixedWidth(qr_pixmap_.width());
                //title->setAlignment(Qt::AlignCenter);
                //title->setText(tr("Server QR Code"));
                //title->setStyleSheet(R"(font-size: 15px;)");
                //layout->addWidget(title);

                auto qr_info = new QLabel(this);
                qr_info->setPixmap(qr_pixmap_);
                //layout->addSpacing(9);
                layout->addWidget(qr_info);
                layout->addStretch();
                machine_code_qr_layout->addLayout(layout);

                int size = 40;
                auto img_path = std::format(":/icons/{}.png", context_->GetIndexByUniqueId());
                auto avatar = new RoundImageDisplay(img_path.c_str(), size, size, size/2);
                avatar->setParent(qr_info);
                avatar->setGeometry((qr_pixmap_.width()-size)/2, (qr_pixmap_.height()-size)/2, size, size);

            }

            // Remote Device
            {
                auto title = new QLabel(this);
                title->setFixedWidth(item_width);
                title->setText(tr("REMOTE DEVICE"));
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 27px; font-weight:700; font-family: ScreenMatrix;)"); //; padding-left: 10px
                left_root->addSpacing(50);
                left_root->addWidget(title, 0, Qt::AlignLeft);
            }

            left_root->addSpacing(18);

            // remote machine code
            {
                auto layout = new NoMarginVLayout();
                layout->addSpacing(10);

                // Machine Code //
                {
                    auto title = new QLabel(this);
                    title->setFixedWidth(item_width);
                    title->setText(tr("Remote Machine Code"));
                    title->setAlignment(Qt::AlignLeft);
                    title->setStyleSheet(R"(font-size: 12px; font-weight:500;)");
                    //layout->addSpacing(2);
                    layout->addWidget(title, 0, Qt::AlignLeft);

                    auto remote_codes = new QLineEdit(this);
                    remote_codes->setFixedWidth(item_width);
                    remote_codes->setFixedHeight(40);
                    //msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    auto uid = QString::fromStdString(tc::SpaceId(context_->GetSysUniqueId()));
                    //msg->setText(uid);
                    //msg->setStyleSheet(R"(font-size: 30px; font-family: ScreenMatrix;)");
                    remote_codes->setStyleSheet(R"(font-size: 22px; font-weight: 700; color: #2979ff;)");
                    layout->addSpacing(5);
                    layout->addWidget(remote_codes, 0, Qt::AlignLeft);
                    left_root->addLayout(layout);
                }

                // Temporary Password
                {
                    layout->addSpacing(18);

                    auto title = new QLabel(this);
                    title->setFixedWidth(item_width);
                    title->setText(tr("Remote Machine Password"));
                    title->setAlignment(Qt::AlignLeft);
                    title->setStyleSheet(R"(font-size: 12px; font-weight:500;)");
                    //layout->addSpacing(2);
                    layout->addWidget(title, 0, Qt::AlignLeft);

                    auto remote_pwd = new QLineEdit(this);
                    remote_pwd->setFixedWidth(item_width);
                    remote_pwd->setFixedHeight(40);
                    //msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    remote_pwd->setText("3FdsArg4");
                    //msg->setStyleSheet(R"(font-size: 30px; font-family: ScreenMatrix;)");
                    remote_pwd->setStyleSheet(R"(font-size: 22px; font-weight: 700; color: #2979ff;)");
                    layout->addSpacing(5);
                    layout->addWidget(remote_pwd, 0, Qt::AlignLeft);
                    layout->addStretch();
                    left_root->addLayout(layout);
                }

                // connect
                {
                    auto btn_conn = new QPushButton();
                    btn_conn->setFixedWidth(item_width);
                    btn_conn->setFixedHeight(40);
                    btn_conn->setText(tr("Connect"));
                    left_root->addSpacing(30);
                    left_root->addWidget(btn_conn);
                }
            }


//            auto label_width = 195;
//            {
//                auto layout = new NoMarginVLayout();
//                left_root->addLayout(layout);
//                int margin_left = 20;
//                // driver status
//                {
//                    auto item_layout = new NoMarginHLayout();
//                    item_layout->addSpacing(margin_left);
//                    auto icon = new QLabel(this);
//                    icon->setFixedSize(38, 38);
//                    icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_game_controller.svg"));
//                    item_layout->addWidget(icon);
//
//                    auto label = new QLabel(this);
//                    label->setFixedSize(label_width, 40);
//                    label->setText(tr("ViGEm Driver Status"));
//                    label->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(label);
//
//                    auto status = new QLabel(this);
//                    lbl_vigem_state_ = status;
//                    status->setAlignment(Qt::AlignCenter);
//                    status->setFixedSize(80, 26);
//                    status->setText("OK");
//                    item_layout->addWidget(status);
//
//                    auto btn_install = new QPushButton(this);
//                    btn_install->setFixedSize(80, 28);
//                    btn_install->setText(tr("INSTALL"));
//                    item_layout->addSpacing(40);
//                    item_layout->addWidget(btn_install);
//
//                    auto btn_remove = new QPushButton(this);
//                    btn_remove->hide();
//                    btn_remove->setFixedSize(80, 28);
//                    btn_remove->setText(tr("REMOVE"));
//                    item_layout->addSpacing(5);
//                    item_layout->addWidget(btn_remove);
//                    item_layout->addStretch();
//
//                    connect(btn_install, &QPushButton::clicked, this, [=, this]() {
//                        auto msg_box = SizedMessageBox::MakeOkCancelBox(tr("Install ViGEm"), tr("Do you want to install ViGEm?"));
//                        if (msg_box->exec() == 0) {
//                            context_->SendAppMessage(MsgInstallViGEm{});
//                        }
//                    });
//
//                    layout->addLayout(item_layout);
//                }
//
//                // server status
//                {
//                    auto item_layout = new NoMarginHLayout();
//                    item_layout->addSpacing(margin_left);
//                    auto icon = new QLabel(this);
//                    icon->setFixedSize(38, 38);
//                    icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_renderer.svg"));
//                    item_layout->addWidget(icon);
//
//                    auto label = new QLabel(this);
//                    label->setFixedSize(label_width, 40);
//                    label->setText(tr("Renderer Status"));
//                    label->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(label);
//
//                    auto status = new QLabel(this);
//                    lbl_renderer_state_ = status;
//                    status->setAlignment(Qt::AlignCenter);
//                    status->setFixedSize(80, 26);
//                    status->setText("OK");
//                    item_layout->addWidget(status);
//
//                    auto btn_restart = new QPushButton(this);
//                    btn_restart->setFixedSize(80, 28);
//                    btn_restart->setText(tr("RESTART"));
//                    item_layout->addSpacing(40);
//                    item_layout->addWidget(btn_restart);
//
//                    auto btn_remove = new QPushButton(this);
//                    btn_remove->hide();
//                    btn_remove->setFixedSize(80, 28);
//                    btn_remove->setText(tr("REMOVE"));
//                    item_layout->addSpacing(5);
//                    item_layout->addWidget(btn_remove);
//                    item_layout->addStretch();
//
//                    connect(btn_restart, &QPushButton::clicked, this, [=, this]() {
//                       // restart
//                        auto msg_box = SizedMessageBox::MakeOkCancelBox(tr("Restart Renderer"), tr("Do you want to restart Renderer?"));
//                        if (msg_box->exec() == 0) {
//                            this->context_->PostTask([=, this]() {
//                                RestartServer();
//                            });
//                        }
//                    });
//
//                    layout->addLayout(item_layout);
//                }
//
//                // service status
//                {
//                    auto item_layout = new NoMarginHLayout();
//                    item_layout->addSpacing(margin_left);
//                    auto icon = new QLabel(this);
//                    icon->setFixedSize(38, 38);
//                    icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_service.svg"));
//                    item_layout->addWidget(icon);
//
//                    auto label = new QLabel(this);
//                    label->setFixedSize(label_width, 40);
//                    label->setText(tr("Service Status"));
//                    label->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(label);
//
//                    auto status = new QLabel(this);
//                    lbl_service_state_ = status;
//                    status->setAlignment(Qt::AlignCenter);
//                    status->setFixedSize(80, 26);
//                    status->setText("OK");
//                    item_layout->addWidget(status);
//
//                    auto btn_install = new QPushButton(this);
//                    btn_install->setFixedSize(80, 28);
//                    btn_install->setText(tr("INSTALL"));
//                    item_layout->addSpacing(40);
//                    item_layout->addWidget(btn_install);
//
////                    auto btn_remove = new QPushButton(this);
////                    btn_remove->setFixedSize(80, 28);
////                    btn_remove->setText(tr("STOP ALL"));
////                    btn_remove->setProperty("class", "danger");
////                    item_layout->addSpacing(5);
////                    item_layout->addWidget(btn_remove);
//                    item_layout->addStretch();
//
//                    connect(btn_install, &QPushButton::clicked, this, [=, this]() {
//                        auto msg_box = SizedMessageBox::MakeOkCancelBox(tr("Install Service"), tr("Do you want to install Service?"));
//                        if (msg_box->exec() == 0) {
//                            this->context_->PostTask([=, this]() {
//                                this->context_->GetServiceManager()->Install();
//                            });
//                        }
//                    });
//
////                    connect(btn_remove, &QPushButton::clicked, this, [=, this]() {
////                        auto msg_box = SizedMessageBox::MakeOkCancelBox(tr("Remove Service"), tr("Do you want to STOP ALL?"));
////                        if (msg_box->exec() == 0) {
////                            this->context_->PostTask([=, this]() {
////                                this->context_->GetServiceManager()->Remove();
////                            });
////                        }
////                    });
//
//                    layout->addLayout(item_layout);
//                }
//
//                auto ips = context_->GetIps();
//                // IPs
//                for (auto& [ip, ip_type] : ips) {
//                    auto item_layout = new NoMarginHLayout();
//                    item_layout->addSpacing(margin_left);
//                    auto icon = new QLabel(this);
//                    icon->setFixedSize(38, 38);
//                    icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_network.svg"));
//                    item_layout->addWidget(icon);
//
//                    auto label = new QLabel(this);
//                    label->setFixedSize(label_width, 40);
//                    label->setText(tr("Detected IP"));
//                    label->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(label);
//
//                    auto value = new QLabel(this);
//                    value->setFixedSize(120, 40);
//                    value->setText(ip.c_str());
//                    value->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(value);
//
//                    auto nt_type = new QLabel(this);
//                    nt_type->setFixedSize(80, 40);
//                    nt_type->setText(ip_type == IPNetworkType::kWired ? "WIRE" : "WIRELESS");
//                    nt_type->setStyleSheet("font-size: 14px;");
//                    item_layout->addSpacing(18);
//                    item_layout->addWidget(nt_type);
//
//                    item_layout->addStretch();
//                    layout->addLayout(item_layout);
//                }
//
//                // http server port
//                {
//                    auto item_layout = new NoMarginHLayout();
//                    item_layout->addSpacing(margin_left);
//                    auto icon = new QLabel(this);
//                    icon->setFixedSize(38, 38);
//                    icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_port.svg"));
//                    item_layout->addWidget(icon);
//
//                    auto label = new QLabel(this);
//                    label->setFixedSize(label_width, 40);
//                    label->setText(tr("Http Port"));
//                    label->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(label);
//
//                    auto value = new QLabel(this);
//                    value->setFixedSize(120, 40);
//                    value->setText(std::to_string(settings_->http_server_port_).c_str());
//                    value->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(value);
//                    item_layout->addStretch();
//                    layout->addLayout(item_layout);
//                }
//
//                // ws server port
//                {
//                    auto item_layout = new NoMarginHLayout();
//                    item_layout->addSpacing(margin_left);
//                    auto icon = new QLabel(this);
//                    icon->setFixedSize(38, 38);
//                    icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_port.svg"));
//                    item_layout->addWidget(icon);
//
//                    auto label = new QLabel(this);
//                    label->setFixedSize(label_width, 40);
//                    label->setText(tr("Websocket Port"));
//                    label->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(label);
//
//                    auto value = new QLabel(this);
//                    value->setFixedSize(120, 40);
//                    value->setText(std::to_string(settings_->ws_server_port_).c_str());
//                    value->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(value);
//                    item_layout->addStretch();
//                    layout->addLayout(item_layout);
//                }
//
//                // GammaRayRender port
//                {
//                    auto item_layout = new NoMarginHLayout();
//                    item_layout->addSpacing(margin_left);
//                    auto icon = new QLabel(this);
//                    icon->setFixedSize(38, 38);
//                    icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_port.svg"));
//                    item_layout->addWidget(icon);
//
//                    auto label = new QLabel(this);
//                    label->setFixedSize(label_width, 40);
//                    label->setText(tr("Streaming Port"));
//                    label->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(label);
//
//                    auto value = new QLabel(this);
//                    value->setFixedSize(120, 40);
//                    value->setText(std::to_string(settings_->network_listen_port_).c_str());
//                    value->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(value);
//                    item_layout->addStretch();
//                    layout->addLayout(item_layout);
//                }
//
//                {
//                    auto item_layout = new NoMarginHLayout();
//                    item_layout->addSpacing(margin_left);
//                    auto icon = new QLabel(this);
//                    icon->setFixedSize(38, 38);
//                    icon->setStyleSheet(GetItemIconStyleSheet(":/icons/ic_spectrum.svg"));
//                    item_layout->addWidget(icon);
//
//                    auto label = new QLabel(this);
//                    label->setFixedSize(label_width, 40);
//                    label->setText(tr("Audio Spectrum"));
//                    label->setStyleSheet("font-size: 14px;");
//                    item_layout->addWidget(label);
//
//                    auto value = new QLabel(this);
//                    value->setFixedSize(label_width, 40);
//                    value->setStyleSheet("font-size: 14px;");
//                    lbl_audio_format_ = value;
//                    item_layout->addWidget(value);
//                    item_layout->addStretch();
//                    layout->addLayout(item_layout);
//
//                    auto sc_layout = new NoMarginHLayout();
//                    sc_layout->addSpacing(margin_left);
//                    spectrum_circle_ = new QtCircle(this);
//                    spectrum_circle_->setFixedSize(400, 140);
//                    sc_layout->addWidget(spectrum_circle_);
//                    layout->addLayout(sc_layout);
//                }
//            }

            left_root->addStretch(120);
        }

        // right part
        {
            auto layout = new QVBoxLayout();
            LayoutHelper::ClearMargins(layout);
            rn_stack_ = new QStackedWidget(this);
            rn_app_ = new RnApp(app_, this);
            //rn_empty_ = new RnEmpty(context_, this);
            //rn_stack_->addWidget(rn_empty_);
            rn_stack_->addWidget(rn_app_);

            content_layout->addWidget(rn_stack_);
        }

        //
        //content_layout->addStretch(100);

        root_layout->addLayout(content_layout);
        setLayout(root_layout);

        rn_stack_->setCurrentIndex(0);

        // messages
//        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
//        msg_listener_->Listen<MsgViGEmState>([=, this](const MsgViGEmState& state) {
//            context_->PostUITask([=, this]() {
//                this->RefreshVigemState(state.ok_);
//            });
//        });
//
//        msg_listener_->Listen<MsgServerAlive>([=, this](const MsgServerAlive& state) {
//            context_->PostUITask([=, this]() {
//                this->RefreshServerState(state.alive_);
//            });
//        });
//
//        msg_listener_->Listen<MsgServiceAlive>([=, this](const MsgServiceAlive& state) {
//            context_->PostUITask([=, this]() {
//                this->RefreshServiceState(state.alive_);
//            });
//        });
//
//        msg_listener_->Listen<MsgGrTimer100>([=, this](const auto& m) {
//            context_->PostUITask([=, this]() {
//                this->RefreshUIEverySecond();
//            });
//        });
//
//        msg_listener_->Listen<AppMsgRestartServer>([=, this](const AppMsgRestartServer& msg) {
//            context_->PostTask([=, this]() {
//                RestartServer();
//            });
//        });
    }

    TabServer::~TabServer() {

    }

    void TabServer::OnTabShow() {
        TabBase::OnTabShow();
    }

    void TabServer::OnTabHide() {
        TabBase::OnTabHide();
    }

    QString TabServer::GetItemIconStyleSheet(const QString &url) {
        QString style = R"(background-image: url(%1);
                        background-repeat: no-repeat;
                        background-position: center;
                    )";
        return style.arg(url);
    }

    void TabServer::RefreshVigemState(bool ok) {
        RefreshIndicatorState(lbl_vigem_state_, ok);
    }

    void TabServer::RefreshServerState(bool ok) {
        RefreshIndicatorState(lbl_renderer_state_, ok);
    }

    void TabServer::RefreshServiceState(bool ok) {
        RefreshIndicatorState(lbl_service_state_, ok);
    }

    void TabServer::RefreshIndicatorState(QLabel* indicator, bool ok) {
        if (ok) {
            indicator->setStyleSheet("font-size: 13px; font-weight: bold; color:#ffffff; background:#00cc00; border-radius:13px");
            indicator->setText("OK");
        } else {
            indicator->setStyleSheet("font-size: 13px; font-weight: bold; color:#ffffff; background:#cc0000; border-radius:13px");
            indicator->setText("ERROR");
        }
    }

    void TabServer::RefreshUIEverySecond() {
        this->lbl_audio_format_->setText(std::format("Format: {}/{}/{}", statistics_->audio_samples_, statistics_->audio_channels_, statistics_->audio_bits_).c_str());
    }

    void TabServer::RestartServer() {
        auto srv_mgr = this->context_->GetRenderController();
        //srv_mgr->StopServer();
        srv_mgr->ReStart();
        this->context_->SendAppMessage(MsgServerAlive {
            .alive_ = false,
        });
    }
}
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
#include <QRegExp>
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_app_messages.h"
#include "tc_common_new/qrcode/qr_generator.h"
#include "tc_qt_widget/widget_helper.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_qt_widget/round_img_display.h"
#include "rn_app.h"
#include "rn_empty.h"
#include "tc_common_new/message_notifier.h"
#include "render_panel/gr_app_messages.h"
#include "tc_common_new/log.h"
#include "qt_circle.h"
#include "render_panel/gr_statistics.h"
#include "render_panel/gr_application.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "render_panel/gr_render_controller.h"
#include "service/service_manager.h"
#include "tc_common_new/uid_spacer.h"
#include "render_panel/devices/stream_content.h"
#include "tc_qt_widget/tc_qr_widget.h"
#include "tc_qt_widget/tc_font_manager.h"
#include "tc_qt_widget/tc_label.h"
#include "tc_qt_widget/tc_pushbutton.h"
#include "tc_qt_widget/tc_image_button.h"

namespace tc
{

    TabServer::TabServer(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        settings_ = GrSettings::Instance();

        UpdateQRCode();

        // root layout
        auto root_layout = new QVBoxLayout();
        WidgetHelper::ClearMargins(root_layout);

        // content layout
        auto content_layout = new QHBoxLayout();
        WidgetHelper::ClearMargins(content_layout);

        auto item_width = 230;

        // left part
        {
            auto left_root = new NoMarginVLayout();

            // This Device
            {
                auto title = new TcLabel(this);
                title->setFixedWidth(item_width);
                title->SetTextId("id_this_device");
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 22px; font-weight:700;)");
                left_root->addSpacing(10);
                left_root->addWidget(title, 0, Qt::AlignLeft);
            }

            auto machine_code_qr_layout = new NoMarginHLayout();
            left_root->addSpacing(18);
            left_root->addLayout(machine_code_qr_layout);
            content_layout->addSpacing(15);
            content_layout->addLayout(left_root);
            content_layout->addSpacing(15);

            // machine code
            {
                auto layout = new NoMarginVLayout();
                layout->addSpacing(10);

                // Machine Code //
                {
                    auto title = new TcLabel(this);
                    title->setFixedWidth(item_width);
                    title->SetTextId("id_device_id");
                    title->setAlignment(Qt::AlignLeft);
                    title->setStyleSheet(R"(font-size: 12px; font-weight:500;)");
                    //layout->addSpacing(2);
                    layout->addWidget(title, 0, Qt::AlignLeft);

                    auto msg = new QLabel(this);
                    lbl_machine_code_ = msg;
                    msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    //auto uid = QString::fromStdString(tc::SpaceId(context_->GetSysUniqueId()));
                    msg->setText(tc::SpaceId("---------").c_str());
                    msg->setStyleSheet(R"(font-size: 22px; font-weight: 700; color: #2979ff;)");
                    layout->addSpacing(5);
                    layout->addWidget(msg, 0, Qt::AlignLeft);
                    machine_code_qr_layout->addLayout(layout);
                }

                // Temporary Password
                {
                    layout->addSpacing(18);

                    auto title = new TcLabel(this);
                    title->setFixedWidth(230);
                    title->SetTextId("id_temporary_password");
                    title->setAlignment(Qt::AlignLeft);
                    title->setStyleSheet(R"(font-size: 12px; font-weight:500;)");
                    //layout->addSpacing(2);
                    layout->addWidget(title, 0, Qt::AlignLeft);

                    auto msg = new QLabel(this);
                    lbl_machine_random_pwd_ = msg;
                    msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    msg->setText("********");
                    msg->setStyleSheet(R"(font-size: 22px; font-weight: 700; color: #2979ff;)");
                    layout->addSpacing(5);
                    layout->addWidget(msg, 0, Qt::AlignLeft);
                    layout->addStretch();
                    machine_code_qr_layout->addLayout(layout);
                }
            }

            {
                auto layout = new NoMarginVLayout();
                layout->addSpacing(10);

                auto qr_info = new TcQRWidget(this);
                qr_info->setFixedSize(171, 171);
                lbl_qr_code_ = qr_info;
                qr_info->SetQRPixmap(qr_pixmap_);
                layout->addWidget(qr_info);
                layout->addStretch();
                machine_code_qr_layout->addLayout(layout);

                int size = 30;
                auto img_path = std::format(":/icons/{}.png", context_->GetIndexByUniqueId());
                auto avatar = new RoundImageDisplay(img_path.c_str(), size, size, size/2);
                avatar->setParent(qr_info);
                avatar->setGeometry((qr_info->width()-size)/2, (qr_info->height()-size)/2, size, size);

            }

            // Remote Device
            {
                auto title = new TcLabel(this);
                title->setFixedWidth(item_width);
                title->SetTextId("id_remote_device");
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 22px; font-weight:700;)");
                left_root->addSpacing(50);
                left_root->addWidget(title, 0, Qt::AlignLeft);
            }

            left_root->addSpacing(18);

            // remote machine code
            {
                auto remote_input_width = 160;
                auto remote_input_layout = new NoMarginHLayout();

                // Machine Code //
                {
                    auto input_layout = new NoMarginVLayout();
                    auto title = new TcLabel(this);
                    title->setFixedWidth(remote_input_width);
                    title->SetTextId("id_remote_device_id");
                    title->setAlignment(Qt::AlignLeft);
                    title->setStyleSheet(R"(font-size: 12px; font-weight:500;)");
                    input_layout->addWidget(title, 0, Qt::AlignLeft);

                    auto remote_codes = new QComboBox(this);
                    remote_codes->setValidator(new QIntValidator(this));
                    remote_codes->setFixedWidth(remote_input_width);
                    remote_codes->setFixedHeight(35);
                    remote_codes->setStyleSheet(R"(font-size: 16px; font-weight: 700; color: #2979ff;)");
                    remote_codes->setEditable(true);
                    remote_codes->addItems({"1", "2", "3"});
                    input_layout->addSpacing(5);
                    input_layout->addWidget(remote_codes, 0, Qt::AlignLeft);
                    remote_input_layout->addLayout(input_layout);
                }

                // Temporary Password
                {
                    remote_input_layout->addSpacing(8);
                    auto input_layout = new NoMarginVLayout();

                    auto title = new TcLabel(this);
                    title->setFixedWidth(remote_input_width);
                    title->SetTextId("id_remote_device_password");
                    title->setAlignment(Qt::AlignLeft);
                    title->setStyleSheet(R"(font-size: 12px; font-weight:500;)");
                    input_layout->addWidget(title, 0, Qt::AlignLeft);

                    auto remote_pwd = new QLineEdit(this);
                    remote_password_ = remote_pwd;
                    remote_pwd->setEchoMode(QLineEdit::Password);
                    remote_pwd->setFixedWidth(remote_input_width);
                    remote_pwd->setFixedHeight(35);
                    remote_pwd->setText("");
                    remote_pwd->setStyleSheet(R"(font-size: 16px; font-weight: 700; color: #2979ff;)");
                    input_layout->addSpacing(5);
                    input_layout->addWidget(remote_pwd, 0, Qt::AlignLeft);
                    remote_input_layout->addLayout(input_layout);

                    auto show_password = new TcImageButton(":/resources/image/ic_key.svg", QSize(16, 16), this);
                    btn_password_echo_change_ = show_password;
                    show_password->SetColor(0xf5f5f5, 0xe9e9e9, 0xd8d8d8);
                    show_password->SetRoundRadius(15);
                    show_password->setFixedSize(22, 22);
                    show_password->SetOnImageButtonClicked([=, this]() {
                        if (remote_pwd->echoMode() == QLineEdit::Password) {
                            remote_pwd->setEchoMode(QLineEdit::Normal);
                        }
                        else {
                            remote_pwd->setEchoMode(QLineEdit::Password);
                        }
                    });
                }

                // connect
                {
                    auto input_layout = new NoMarginVLayout();

                    auto title = new TcLabel(this);
                    title->setFixedWidth(1);
                    title->setAlignment(Qt::AlignLeft);
                    title->setStyleSheet(R"(font-size: 12px; font-weight:500;)");
                    input_layout->addWidget(title, 0, Qt::AlignLeft);

                    auto btn_conn = new TcPushButton();
                    btn_conn->setFixedWidth(80);
                    btn_conn->setFixedHeight(35);
                    btn_conn->SetTextId("id_connect");
                    input_layout->addWidget(btn_conn);
                    remote_input_layout->addSpacing(8);
                    remote_input_layout->addLayout(input_layout);

                }

                left_root->addLayout(remote_input_layout);
            }
            left_root->addStretch(120);
        }

        // clients
        {
            stream_content_ = new StreamContent(context_, this);
            stream_content_->setMinimumWidth(780);
            content_layout->addWidget(stream_content_);
        }

        root_layout->addLayout(content_layout);
        setLayout(root_layout);

        // set client id by settings
        if (!settings_->device_id_.empty() && !settings_->device_random_pwd_.empty()) {
            lbl_machine_code_->setText(tc::SpaceId(settings_->device_id_).c_str());
            lbl_machine_random_pwd_->setText(settings_->device_random_pwd_.c_str());
        }

        RegisterMessageListener();
    }

    TabServer::~TabServer() = default;

    void TabServer::OnTabShow() {
        TabBase::OnTabShow();
    }

    void TabServer::OnTabHide() {
        TabBase::OnTabHide();
    }

    void TabServer::RegisterMessageListener() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgRequestedNewDevice>([=, this](const MsgRequestedNewDevice& msg) {
            context_->PostUITask([=, this]() {
                lbl_machine_code_->setText(tc::SpaceId(msg.device_id_).c_str());
                lbl_machine_random_pwd_->setText(msg.device_random_pwd_.c_str());
                this->UpdateQRCode();
            });
        });
    }

    void TabServer::UpdateQRCode() {
        auto broadcast_msg = context_->MakeBroadcastMessage();
        qr_pixmap_ = QrGenerator::GenQRPixmap(broadcast_msg.c_str(), -1);
        if (lbl_qr_code_) {
            lbl_qr_code_->SetQRPixmap(qr_pixmap_);
        }
    }

    void TabServer::resizeEvent(QResizeEvent *event) {
        auto r_pos = remote_password_->pos();
        auto r_width = remote_password_->width();
        auto r_height = remote_password_->height();
        auto btn_width = btn_password_echo_change_->width();
        auto btn_height = btn_password_echo_change_->height();
        btn_password_echo_change_->setGeometry(r_pos.x() + r_width - 5 - btn_width, r_pos.y() + (r_height-btn_height)/2, btn_width, btn_height);
    }
}
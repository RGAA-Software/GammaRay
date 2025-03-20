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
#include "render_panel/gr_app_messages.h"
#include "tc_common_new/qrcode/qr_generator.h"
#include "tc_qt_widget/layout_helper.h"
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
#include "client/ct_client_context.h"
#include "client/ui/stream_content.h"
#include "tc_qt_widget/tc_qr_widget.h"

namespace tc
{

    TabServer::TabServer(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        settings_ = GrSettings::Instance();
        // client
        client_ctx_ = std::make_shared<ClientContext>("ui.embed");
        // update when new device id requested
        client_ctx_->SetDeviceId(settings_->device_id_);
        client_ctx_->Init(false);

        UpdateQRCode();

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
                title->setText(tr("This Device"));
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 25px; font-weight:700;)");
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
                    lbl_machine_code_ = msg;
                    msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    //auto uid = QString::fromStdString(tc::SpaceId(context_->GetSysUniqueId()));
                    msg->setText(tc::SpaceId("---------").c_str());
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
                    lbl_machine_random_pwd_ = msg;
                    msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    msg->setText("********");
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
                auto title = new QLabel(this);
                title->setFixedWidth(item_width);
                title->setText(tr("Remote Device"));
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 25px; font-weight:700;)");
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
                    layout->addWidget(title, 0, Qt::AlignLeft);

                    auto remote_codes = new QLineEdit(this);
                    remote_codes->setFixedWidth(item_width);
                    remote_codes->setFixedHeight(40);
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
                    layout->addWidget(title, 0, Qt::AlignLeft);

                    auto remote_pwd = new QLineEdit(this);
                    remote_pwd->setFixedWidth(item_width);
                    remote_pwd->setFixedHeight(40);
                    remote_pwd->setText("");
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

            left_root->addStretch(120);
        }

        // clients
        {
            stream_content_ = new StreamContent(client_ctx_, this);
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
                client_ctx_->SetDeviceId(settings_->device_id_);

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
}
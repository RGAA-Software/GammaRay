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
#include "tc_qt_widget/tc_password_input.h"
#include "tc_spvr_client/spvr_manager.h"
#include "tc_common_new/base64.h"
#include "tc_dialog.h"
#include "render_panel/devices/running_stream_manager.h"
#include "render_panel/devices/stream_db_manager.h"

namespace tc
{

    TabServer::TabServer(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        settings_ = GrSettings::Instance();
        running_stream_mgr_ = context_->GetRunningStreamManager();
        stream_db_mgr_ = context_->GetStreamDBManager();

        UpdateQRCode();

        // root layout
        auto root_layout = new QVBoxLayout();
        WidgetHelper::ClearMargins(root_layout);

        // content layout
        auto content_layout = new QHBoxLayout();
        WidgetHelper::ClearMargins(content_layout);

        auto item_width = 210;

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
            content_layout->addSpacing(5);

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

                int size = 22;
                auto img_path = std::format(":/icons/{}.png", context_->GetIndexByUniqueId());
                auto avatar = new RoundImageDisplay(img_path.c_str(), size, size, 4);
                avatar->setParent(qr_info);
                avatar->setGeometry((qr_info->width()-size)/2+1, (qr_info->height()-size)/2+1, size, size);

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
                    remote_devices_ = remote_codes;
                    remote_codes->setValidator(new QIntValidator(this));
                    remote_codes->setFixedWidth(remote_input_width);
                    remote_codes->setFixedHeight(35);
                    remote_codes->setStyleSheet(R"(font-size: 16px; font-weight: 700; color: #2979ff;)");
                    remote_codes->setEditable(true);

                    recent_streams_ = stream_db_mgr_->GetStreamsSortByCreatedTime(1, 5, false);
                    for (auto& stream : recent_streams_) {
                        remote_codes->addItem(stream.remote_device_id_.c_str());
                    }

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

                    password_input_ = new TcPasswordInput(this);
                    password_input_->setFixedSize(remote_input_width, 35);
                    if (!recent_streams_.empty()) {
                        auto first_item = recent_streams_.at(0);
                        auto item_pwd = first_item.remote_device_random_pwd_;
                        password_input_->SetPassword(item_pwd.c_str());
                    }
                    input_layout->addSpacing(5);
                    input_layout->addWidget(password_input_, 0, Qt::AlignLeft);
                    remote_input_layout->addLayout(input_layout);
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

                    connect(btn_conn, &QPushButton::clicked, this, [=, this]() {
                        auto remote_device_id = remote_devices_->currentText().replace(" ", "").trimmed().toStdString();
                        auto remote_password = password_input_->GetPassword().toStdString();
                        auto srv_remote_device_id = "server_" + remote_device_id;
                        auto spvr_mgr = context_->GetSpvrManager();
                        auto r = spvr_mgr->GetDeviceInfo(srv_remote_device_id);
                        if (!r) {
                            LOGE("Get device info for: {} failed: {}", srv_remote_device_id, SpvrError2String(r.error()));
                            auto dg = TcDialog::Make(tr("Error"), tr("Can't get remote device information."), nullptr);
                            dg->Show();
                            return;
                        }
                        auto remote_device_info = r.value();
                        LOGI("Remote device info: id: {}, relay host: {}, port: {}",
                             srv_remote_device_id, remote_device_info.relay_server_ip_, remote_device_info.relay_server_port_);

                        StreamItem item;
                        item.stream_id_ = "id_" + remote_device_id;
                        item.stream_name_ = remote_device_id;
                        item.stream_host_ = remote_device_info.relay_server_ip_;
                        item.stream_port_ = remote_device_info.relay_server_port_;
                        item.encode_bps_ = 0;
                        item.encode_fps_ = 0;
                        item.network_type_ = kStreamItemNtTypeRelay;
                        item.remote_device_id_ = remote_device_id;
                        item.remote_device_random_pwd_ = remote_password;
                        item.remote_device_safety_pwd_ = remote_password;
                        context_->SendAppMessage(StreamItemAdded {
                            .item_ = item,
                        });

                        running_stream_mgr_->StartStream(item);
                    });
                }

                left_root->addLayout(remote_input_layout);
            }
            left_root->addStretch(120);
        }

        // clients
        {
            stream_content_ = new StreamContent(context_, this);
            stream_content_->setMinimumWidth(800);
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
        TabBase::resizeEvent(event);
    }
}
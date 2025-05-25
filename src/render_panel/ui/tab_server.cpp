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
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QClipboard>
#include <QApplication>
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
#include "render_panel/devices/device_api.h"
#include "render_panel/gr_workspace.h"

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

        // title margin
        root_layout->addSpacing(kTabContentMarginTop);

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

            // Connect Information
            {
                auto title = new TcLabel(this);
                title->setFixedWidth(item_width+50);
                title->SetTextId("id_connect_information");
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 22px; font-weight:700;)");
                left_root->addSpacing(20);
                left_root->addWidget(title, 0, Qt::AlignLeft);
            }

            // Temporary Password
            {
                left_root->addSpacing(18);

                auto title = new TcLabel(this);
                title->setFixedWidth(230);
                title->SetTextId("id_detailed_information");
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 12px; font-weight:500;)");
                //layout->addSpacing(2);
                left_root->addWidget(title, 0, Qt::AlignLeft);

                auto layout = new NoMarginHLayout();
                auto msg = new QLineEdit(this);
                lbl_detailed_info_ = msg;
                msg->setAlignment(Qt::AlignLeft);
                msg->setFixedWidth(330);
                msg->setFixedHeight(35);
                auto info = std::format("gammaray://{}", Base64::Base64Encode(context_->MakeBroadcastMessage()));
                msg->setText(info.c_str());
                msg->setCursorPosition(0);
                msg->setStyleSheet(R"(font-size: 12px; padding-left: 5px; font-weight: 500; color: #2979ff;)");
                msg->setEnabled(false);
                layout->addWidget(msg);

                layout->addSpacing(8);

                auto btn_conn = new TcPushButton();
                btn_conn->setText("Copy");
                btn_conn->setFixedWidth(80);
                btn_conn->setFixedHeight(35);
                layout->addWidget(btn_conn);
                connect(btn_conn, &QPushButton::clicked, this, [=, this]() {
                    QClipboard* clipboard = QApplication::clipboard();
                    clipboard->setText(msg->text());
//                    TcDialog dlg("Tips", "Connection info has been written to the clipboard.");
//                    dlg.exec();

                    context_->NotifyAppMessage("Copy Success", "Information has been written to the Clipboard");
                });

                left_root->addSpacing(5);
                left_root->addLayout(layout);
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
                        remote_codes->addItem(stream->remote_device_id_.c_str());
                    }

                    connect(remote_codes, &QComboBox::currentTextChanged, this, [this](const QString& text) {
                        std::string password = "";
                        for (auto& item : recent_streams_) {
                            if (item->remote_device_id_ == text.toStdString()) {
                                if (!item->remote_device_safety_pwd_.empty()) {
                                    password = item->remote_device_safety_pwd_;
                                }
                                else if (!item->remote_device_random_pwd_.empty()) {
                                    password = item->remote_device_random_pwd_;
                                }
                            }
                        }
                        password_input_->SetPassword(password.c_str());
                    });

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
                        auto item_pwd = first_item->remote_device_random_pwd_;
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
                        // verify my self
                        if (!grApp->CheckLocalDeviceInfoWithPopup()) {
                            return;
                        }

                        auto remote_device_id = remote_devices_->currentText().replace(" ", "").trimmed().toStdString();
                        auto input_password = password_input_->GetPassword().toStdString();

                        // query device in database
                        auto opt_remote_device = stream_db_mgr_->GetStreamByRemoteDeviceId(remote_device_id);
                        auto already_exist_remote_device = opt_remote_device.has_value();

                        std::string random_password;
                        std::string safety_password;
                        // this device is already in database
                        if (already_exist_remote_device) {
                            auto remote_device = opt_remote_device.value();
                            // its passwords
                            random_password = remote_device->remote_device_random_pwd_;
                            safety_password = remote_device->remote_device_safety_pwd_;

                            // the input password is not equals to random password, neither the safety password
                            // this is a new password, will determine its type
                            if (input_password != random_password && input_password != safety_password) {
                                random_password = input_password;
                                safety_password = input_password;
                            }
                        }
                        else {
                            // a new device, 
                            random_password = input_password;
                            safety_password = input_password;
                        }

                        // get device's relay server info
                        auto srv_remote_device_id = "server_" + remote_device_id;
                        auto spvr_mgr = context_->GetSpvrManager();
                        auto relay_result = spvr_mgr->GetRelayDeviceInfo(srv_remote_device_id);
                        if (!relay_result) {
                            LOGE("Get device info for: {} failed: {}", srv_remote_device_id, SpvrError2String(relay_result.error()));
                            TcDialog dialog(tr("Error"), tr("Can't get remote device information."), grWorkspace.get());
                            dialog.exec();
                            return;
                        }
                        auto relay_device_info = relay_result.value();
                        LOGI("Remote device info: id: {}, relay host: {}, port: {}",
                             srv_remote_device_id, relay_device_info.relay_server_ip_, relay_device_info.relay_server_port_);

                        // verify in profile server
                        auto verify_result = DeviceApi::VerifyDeviceInfo(remote_device_id, random_password, safety_password);
                        if (verify_result == DeviceVerifyResult::kVfNetworkFailed) {
                            TcDialog dialog("Connect Failed", "Can't access server.", grWorkspace.get());
                            dialog.exec();
                            return;
                        }
                        if (verify_result != DeviceVerifyResult::kVfSuccessRandomPwd && verify_result != DeviceVerifyResult::kVfSuccessSafetyPwd) {
                            TcDialog dialog("Connect Failed", "Password is invalid, please check it.", grWorkspace.get());
                            dialog.exec();
                            return;
                        }

                        std::shared_ptr<StreamItem> item = std::make_shared<StreamItem>();
                        item->stream_id_ = "id_" + remote_device_id;
                        item->stream_name_ = remote_device_id;
                        item->stream_host_ = relay_device_info.relay_server_ip_;
                        item->stream_port_ = relay_device_info.relay_server_port_;
                        item->encode_bps_ = 0;
                        item->encode_fps_ = 0;
                        item->network_type_ = kStreamItemNtTypeRelay;
                        item->remote_device_id_ = remote_device_id;
                        if (verify_result == DeviceVerifyResult::kVfSuccessRandomPwd) {
                            item->remote_device_random_pwd_ = random_password;
                        }
                        else {
                            item->remote_device_safety_pwd_ = safety_password;
                        }
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

        if (lbl_detailed_info_) {
            auto info = std::format("gammaray://{}", Base64::Base64Encode(context_->MakeBroadcastMessage()));
            lbl_detailed_info_->setText(info.c_str());
        }
    }

    void TabServer::resizeEvent(QResizeEvent *event) {
        TabBase::resizeEvent(event);
    }
}
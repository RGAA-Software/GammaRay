//
// Created by RGAA on 2024/4/9.
//

#include "tab_server.h"
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
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
#include <QCheckBox>
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
#include "tc_common_new/md5.h"
#include "tc_common_new/log.h"
#include "qt_circle.h"
#include "tc_dialog.h"
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
#include "tc_qt_widget/tc_circle_indicator.h"
#include "tc_spvr_client/spvr_device_api.h"
#include "tc_spvr_client/spvr_device.h"
#include "tc_common_new/base64.h"
#include "tc_common_new/tc_aes.h"
#include "render_panel/devices/running_stream_manager.h"
#include "render_panel/database/stream_db_operator.h"
#include "render_panel/gr_workspace.h"
#include "relay_message.pb.h"
#include "tc_profile_client/profile_api.h"
#include "render_panel/companion/panel_companion.h"
#include "client/ct_stream_item_net_type.h"
#include "render_panel/gr_statistics.h"

namespace tc
{

    TabServer::TabServer(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        settings_ = GrSettings::Instance();
        stat_ = GrStatistics::Instance();
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
                title->setStyleSheet(R"(font-size: 21px; font-weight:700;)");
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

                    auto code_layout = new NoMarginHLayout();

                    auto msg = new QLabel(this);
                    msg->setFixedWidth(140);
                    lbl_machine_code_ = msg;
                    msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    //auto uid = QString::fromStdString(tc::SpaceId(context_->GetSysUniqueId()));
                    msg->setText(tc::SpaceId("---------").c_str());
                    msg->setStyleSheet(R"(font-size: 21px; font-weight: 700; color: #2979ff;)");
                    code_layout->addWidget(msg);

                    auto btn_cpy = new TcImageButton(":/resources/image/ic_copy.svg", QSize(20, 20));
                    btn_cpy->SetColor(0xffffff, 0xdddddd, 0xbbbbbb);
                    btn_cpy->SetRoundRadius(15);
                    btn_cpy->setFixedSize(30, 30);
                    code_layout->addSpacing(10);
                    code_layout->addWidget(btn_cpy, 0, Qt::AlignVCenter);
                    code_layout->addStretch();

                    layout->addSpacing(5);
                    layout->addLayout(code_layout);
                    machine_code_qr_layout->addLayout(layout);

                    btn_cpy->SetOnImageButtonClicked([=, this]() {
                        if (settings_->GetDeviceId().empty()) {
                            return;
                        }
                        QClipboard* clipboard = QApplication::clipboard();
                        clipboard->setText(msg->text());
                        context_->NotifyAppMessage(tcTr("id_copy_success"), tcTr("id_copy_success_clipboard"));
                    });
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

                    auto pwd_layout = new NoMarginHLayout();
                    auto msg = new QLabel(this);
                    msg->setFixedWidth(140);
                    lbl_machine_random_pwd_ = msg;
                    msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    msg->setText("********");
                    msg->setStyleSheet(R"(font-size: 21px; font-weight: 700; color: #2979ff;)");
                    pwd_layout->addWidget(msg);

                    auto btn_refresh = new TcImageButton(":/resources/image/ic_refresh.svg", QSize(20, 20));
                    btn_refresh->SetColor(0xffffff, 0xdddddd, 0xbbbbbb);
                    btn_refresh->SetRoundRadius(15);
                    btn_refresh->setFixedSize(30, 30);
                    pwd_layout->addSpacing(10);
                    pwd_layout->addWidget(btn_refresh, 0, Qt::AlignVCenter);

                    auto btn_hide_pwd = new TcImageButton(":/resources/image/ic_pwd_visibility_on.svg", ":/resources/image/ic_pwd_visibility_off.svg", QSize(20, 20));
                    btn_hide_random_pwd_ = btn_hide_pwd;
                    btn_hide_pwd->SetColor(0xffffff, 0xdddddd, 0xbbbbbb);
                    btn_hide_pwd->SetRoundRadius(15);
                    btn_hide_pwd->setFixedSize(30, 30);
                    pwd_layout->addSpacing(2);
                    pwd_layout->addWidget(btn_hide_pwd, 0, Qt::AlignVCenter);

                    pwd_layout->addStretch();

                    layout->addSpacing(5);
                    //layout->addWidget(msg, 0, Qt::AlignLeft);
                    layout->addLayout(pwd_layout);
                    layout->addStretch();
                    machine_code_qr_layout->addLayout(layout);

                    // event
                    btn_refresh->SetOnImageButtonClicked([=, this]() {
                        if (settings_->GetDeviceId().empty()) {
                            return;
                        }
                        context_->PostTask([=, this]() {
                            auto opt_device = spvr::SpvrDeviceApi::UpdateRandomPwd(settings_->GetSpvrServerHost(),
                                                                             settings_->GetSpvrServerPort(),
                                                                             grApp->GetAppkey(),
                                                                             settings_->GetDeviceId());
                            if (!opt_device.has_value()) {
                                LOGE("Refresh random password failed, code: {}", (int)opt_device.error());
                                return;
                            }
                            auto device = opt_device.value();
                            if (!device) {
                                LOGE("Refresh random password failed, nullptr.");
                                return;
                            }
                            settings_->SetDeviceId(device->device_id_);
                            settings_->SetDeviceRandomPwd(device->gen_random_pwd_);

                            context_->SendAppMessage(MsgRandomPasswordUpdated {
                                .device_id_ = device->device_id_,
                                .device_random_pwd_ = device->gen_random_pwd_,
                            });

                            context_->SendAppMessage(MsgSyncSettingsToRender{});
                        });
                    });

                    btn_hide_pwd->SetOnImageButtonClicked([=, this]() {
                        if (settings_->GetDeviceRandomPwd().empty()) {
                            return;
                        }
                        settings_->SetDisplayRandomPwd(!settings_->IsDisplayRandomPwd());
                        SetDeviceRandomPwdVisibility();
                    });
                }
            }

            {
                auto layout = new NoMarginVLayout();

                auto qr_info = new TcQRWidget(this);
                //qr_info->setFixedSize(171, 171);
                lbl_qr_code_ = qr_info;
                lbl_qr_code_->setFixedSize(qr_pixmap_.width()*3, qr_pixmap_.height()*3);
                qr_info->SetQRPixmap(qr_pixmap_);
                layout->addWidget(qr_info);
                layout->addStretch();
                machine_code_qr_layout->addLayout(layout);

                int size = 18;
                //auto img_path = std::format(":/icons/{}.png", context_->GetIndexByUniqueId());
                auto img_path = ":/resources/tc_icon.png";
                auto avatar = new RoundImageDisplay(img_path, size, size, 4);
                qr_avatar_ = avatar;
                avatar->setParent(qr_info);
                avatar->setGeometry((qr_info->width()-size)/2, (qr_info->height()-size)/2, size, size);
            }

            // Connect Information
            {
                auto title = new TcLabel(this);
                title->setFixedWidth(item_width+50);
                title->SetTextId("id_connect_information");
                title->setAlignment(Qt::AlignLeft);
                title->setStyleSheet(R"(font-size: 21px; font-weight:700;)");
                left_root->addSpacing(20);
                left_root->addWidget(title, 0, Qt::AlignLeft);
            }

            // link:// information
            {
                left_root->addSpacing(18);

                {
                    auto layout = new NoMarginHLayout();
                    layout->setAlignment(Qt::AlignVCenter);

                    auto title = new TcLabel(this);
                    title->setFixedWidth(160);
                    title->SetTextId("id_detailed_information");
                    title->setAlignment(Qt::AlignLeft);
                    title->setStyleSheet(R"(font-size: 12px; font-weight:500;)");
                    layout->addWidget(title, 0, Qt::AlignLeft);

                    left_root->addLayout(layout);
                }

                auto layout = new NoMarginHLayout();
                auto msg = new QLineEdit(this);
                lbl_detailed_info_ = msg;
                msg->setAlignment(Qt::AlignLeft);
                msg->setFixedWidth(330);
                msg->setFixedHeight(35);
                auto info = std::format("link://{}", Base64::Base64Encode(context_->MakeBroadcastMessage()));
                msg->setText(info.c_str());
                msg->setCursorPosition(0);
                msg->setStyleSheet(R"(font-size: 12px; padding-left: 5px; font-weight: 500; color: #2979ff;)");
                msg->setEnabled(false);
                layout->addWidget(msg);

                layout->addSpacing(8);

                auto btn_conn = new TcPushButton();
                btn_conn->SetTextId("id_copy");
                btn_conn->setFixedWidth(80);
                btn_conn->setFixedHeight(35);
                layout->addWidget(btn_conn);
                connect(btn_conn, &QPushButton::clicked, this, [=, this]() {
                    QClipboard* clipboard = QApplication::clipboard();
                    clipboard->setText(msg->text());
                    context_->NotifyAppMessage(tcTr("id_copy_success"), tcTr("id_copy_success_clipboard"));
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
                title->setStyleSheet(R"(font-size: 21px; font-weight:700;)");
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
                        std::string random_password;
                        std::string safety_password;

                        random_password = input_password;
                        safety_password = input_password;

                        if (remote_device_id.empty()) {
                            return;
                        }

                        // assume the device in the same relay server, so we use the settings' relay server info
                        // get device's relay server info
                        auto relay_host = settings_->GetRelayServerHost();
                        auto relay_port = settings_->GetRelayServerPort();
                        auto relay_appkey = grApp->GetAppkey();
                        auto relay_device_info = context_->GetRelayServerSideDeviceInfo(relay_host, relay_port, relay_appkey, remote_device_id);
                        if (relay_device_info == nullptr) {
                            return;
                        }

                        // verify in profile server
                        auto verify_result = ProfileApi::VerifyDeviceInfo(settings_->GetSpvrServerHost(),
                                                                          settings_->GetSpvrServerPort(),
                                                                          remote_device_id,
                                                                          MD5::Hex(random_password),
                                                                          MD5::Hex(safety_password),
                                                                          grApp->GetAppkey());
                        if (verify_result == ProfileVerifyResult::kVfNetworkFailed) {
                            TcDialog dialog(tcTr("id_connect_failed"), tcTr("id_profile_network_unavailable"), grWorkspace.get());
                            dialog.exec();
                            return;
                        }
                        if (verify_result != ProfileVerifyResult::kVfSuccessRandomPwd
                            && verify_result != ProfileVerifyResult::kVfSuccessSafetyPwd
                            && verify_result != ProfileVerifyResult::kVfSuccessAllPwd) {
                            TcDialog dialog(tcTr("id_connect_failed"), tcTr("id_password_invalid_msg"), grWorkspace.get());
                            dialog.exec();
                            return;
                        }

                        std::shared_ptr<spvr::SpvrStream> item = std::make_shared<spvr::SpvrStream>();
                        item->stream_id_ = "id_" + remote_device_id;
                        item->stream_name_ = remote_device_id;
                        item->stream_host_ = "";
                        item->stream_port_ = 0;
                        item->relay_host_ = relay_device_info->relay_server_ip();
                        item->relay_port_ = relay_device_info->relay_server_port();
                        item->relay_appkey_ = grApp->GetAppkey();
                        item->encode_bps_ = 0;
                        item->encode_fps_ = 0;
                        item->remote_device_id_ = remote_device_id;
                        item->clipboard_enabled_ = true;
                        item->bg_color_ = 0xffffff;
                        if (verify_result == ProfileVerifyResult::kVfSuccessRandomPwd
                            || verify_result == ProfileVerifyResult::kVfSuccessAllPwd) {
                            item->remote_device_random_pwd_ = random_password;
                        }
                        else if (verify_result == ProfileVerifyResult::kVfSuccessSafetyPwd
                            || verify_result == ProfileVerifyResult::kVfSuccessAllPwd) {
                            item->remote_device_safety_pwd_ = safety_password;
                        }
                        context_->SendAppMessage(StreamItemAdded {
                            .item_ = item,
                        });

                        running_stream_mgr_->StartStream(item, kStreamItemNtTypeRelay);
                    });
                }

                left_root->addLayout(remote_input_layout);
            }
            left_root->addStretch(120);

            // server status of myself
            {
                auto layout = new NoMarginHLayout();
                layout->setAlignment(Qt::AlignHCenter);
                auto label_size = QSize(75, 30);
                auto indicator_size = QSize(14, 14);
                {
                    // indicator
                    auto indicator = new TcCircleIndicator(this);
                    spvr_indicator_ = indicator;
                    indicator->setFixedSize(indicator_size);
                    layout->addWidget(indicator);

                    layout->addSpacing(5);

                    // text
                    auto text = new TcLabel(this);
                    text->setFixedSize(label_size);
                    text->SetTextId("id_manager_server");
                    layout->addWidget(text);
                }

                {
                    auto indicator = new TcCircleIndicator(this);
                    relay_indicator_ = indicator;
                    indicator->setFixedSize(indicator_size);
                    layout->addWidget(indicator);

                    layout->addSpacing(5);

                    // text
                    auto text = new TcLabel(this);
                    text->setFixedSize(label_size);
                    text->SetTextId("id_relay_server");
                    layout->addWidget(text);
                }

                {
                    auto indicator = new TcCircleIndicator(this);
                    relay_ft_indicator_ = indicator;
                    indicator->setFixedSize(indicator_size);
                    layout->addWidget(indicator);

                    layout->addSpacing(5);

                    // text
                    auto text = new TcLabel(this);
                    text->setFixedSize(label_size);
                    text->SetTextId("id_relay_ft_server");
                    layout->addWidget(text);
                }


                layout->addStretch();

                left_root->addLayout(layout);
                left_root->addSpacing(10);
            }
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
        if (!settings_->GetDeviceId().empty() && !settings_->GetDeviceRandomPwd().empty()) {
            lbl_machine_code_->setText(tc::SpaceId(settings_->GetDeviceId()).c_str());
            //lbl_machine_random_pwd_->setText(settings_->GetDeviceRandomPwd().c_str());
            SetDeviceRandomPwdVisibility();
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

        // new device created
        msg_listener_->Listen<MsgRequestedNewDevice>([=, this](const MsgRequestedNewDevice& msg) {
            context_->PostUITask([=, this]() {
                lbl_machine_code_->setText(tc::SpaceId(msg.device_id_).c_str());
                //lbl_machine_random_pwd_->setText(msg.device_random_pwd_.c_str());
                SetDeviceRandomPwdVisibility();
                this->UpdateQRCode();
            });
        });

        // random password updated
        msg_listener_->Listen<MsgRandomPasswordUpdated>([=, this](const MsgRandomPasswordUpdated& msg) {
            context_->PostUITask([=, this]() {
                lbl_machine_code_->setText(tc::SpaceId(msg.device_id_).c_str());
                //lbl_machine_random_pwd_->setText(msg.device_random_pwd_.c_str());
                SetDeviceRandomPwdVisibility();
                this->UpdateQRCode();
            });
        });

        // program data cleared
        msg_listener_->Listen<MsgForceClearProgramData>([=, this](const MsgForceClearProgramData& msg) {
            context_->PostUITask([=, this]() {
                lbl_machine_code_->setText(tc::SpaceId("---------").c_str());
                SetDeviceRandomPwdVisibility();
                this->UpdateQRCode();
            });
        });

        msg_listener_->Listen<MsgGrTimer1S>([=, this](const MsgGrTimer1S& m) {
            context_->PostUITask([=, this]() {
                UpdateServerState();
            });
        });
    }

    void TabServer::UpdateQRCode() {
        auto broadcast_msg = context_->MakeBroadcastMessage();

        // companion
        if (grApp->GetCompanion()) {
            std::vector<uint8_t> enc_data;
            if (grApp->GetCompanion()->EncQRCode(broadcast_msg, enc_data)) {
                broadcast_msg = Base64::Base64Encode(enc_data.data(), enc_data.size());
            }
        }

        qr_pixmap_ = QrGenerator::GenQRPixmap(broadcast_msg.c_str(), -1);
        LOGI("QR str: {}", broadcast_msg);
        LOGI("QR pixmap size: {}x{}", qr_pixmap_.width(), qr_pixmap_.height());
        //qr_pixmap_ = qr_pixmap_.scaled(60, 60, Qt::KeepAspectRatio, Qt::FastTransformation);
        if (lbl_qr_code_) {
            lbl_qr_code_->setFixedSize(qr_pixmap_.width()*3, qr_pixmap_.height()*3);
            lbl_qr_code_->SetQRPixmap(qr_pixmap_);
            qr_avatar_->setGeometry((lbl_qr_code_->width()-qr_avatar_->width())/2,
                                    (lbl_qr_code_->height()-qr_avatar_->height())/2,
                                    qr_avatar_->width(),
                                    qr_avatar_->height());
        }

        if (lbl_detailed_info_) {
            auto info = std::format("link://{}", Base64::Base64Encode(context_->MakeBroadcastMessage()));
            lbl_detailed_info_->setText(info.c_str());
            lbl_detailed_info_->setCursorPosition(0);
        }
    }

    void TabServer::resizeEvent(QResizeEvent *event) {
        TabBase::resizeEvent(event);
    }

    void TabServer::SetDeviceRandomPwdVisibility() {
        if (settings_->IsDisplayRandomPwd() && !settings_->GetDeviceRandomPwd().empty()) {
            lbl_machine_random_pwd_->setText(settings_->GetDeviceRandomPwd().c_str());
            btn_hide_random_pwd_->ToImage1();
        }
        else {
            lbl_machine_random_pwd_->setText("********");
            btn_hide_random_pwd_->ToImage2();
        }
    }

    void TabServer::UpdateServerState() {
        bool spvr_client_alive = grApp->IsSpvrClientAlive();
        spvr_indicator_->SetState(spvr_client_alive ? TcCircleIndicator::State::kOk : TcCircleIndicator::State::kError);
        auto device_id = settings_->GetDeviceId();
        if (!device_id.empty()) {
            auto current_ts = TimeUtil::GetCurrentTimestamp();
            auto max_duration = 3500;
            {
                auto sid = "server_" + device_id;
                auto ts = stat_->GetRelayLastUpdateTimestamp(sid);
                //LOGI("relay alive: {}, ts: {}, diff: {}ms", sid, ts, (current_ts - ts));
                if (current_ts - ts < max_duration) {
                    // alive
                    relay_indicator_->SetState(TcCircleIndicator::State::kOk);
                }
                else {
                    relay_indicator_->SetState(TcCircleIndicator::State::kError);
                }
            }
            {
                auto sid = "ft_server_" + device_id;
                auto ts = stat_->GetRelayLastUpdateTimestamp(sid);
                if (current_ts - ts < max_duration) {
                    // alive
                    relay_ft_indicator_->SetState(TcCircleIndicator::State::kOk);
                }
                else {
                    relay_ft_indicator_->SetState(TcCircleIndicator::State::kError);
                }
            }
        }
        else {
            relay_indicator_->SetState(TcCircleIndicator::State::kError);
            relay_ft_indicator_->SetState(TcCircleIndicator::State::kError);
        }
    }
}
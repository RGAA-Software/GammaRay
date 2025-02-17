//
// Created by RGAA on 2024-06-10.
//

#include "st_network.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_settings.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "tc_common_new/win32/dxgi_mon_detector.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/win32/audio_device_helper.h"
#include "render_panel/gr_app_messages.h"
#include "tc_common_new/ip_util.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>
#include <QFileDialog>
#include <Windows.h>
#include <Mmsystem.h>
#include <SetupAPI.h>
#include <devguid.h>
#include <RegStr.h>
#include <initguid.h>

namespace tc
{

    StNetwork::StNetwork(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent){
        auto root_layout = new NoMarginHLayout();
        auto column1_layout = new NoMarginVLayout();
        root_layout->addLayout(column1_layout);

        auto column2_layout = new NoMarginVLayout();
        root_layout->addSpacing(40);
        root_layout->addLayout(column2_layout);

        root_layout->addStretch();

        // segment encoder
        auto tips_label_width = 220;
        auto tips_label_height = 35;
        auto tips_label_size = QSize(tips_label_width, tips_label_height);
        auto input_size = QSize(240, tips_label_height);

        {
            auto segment_layout = new NoMarginVLayout();
            {
                // title
                auto label = new QLabel(this);
                label->setText(tr("Network Settings"));
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            // Network type
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("WebSocket"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                cb_websocket_ = edit;
                edit->setFixedSize(input_size);
                edit->setEnabled(true);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->websocket_enabled_ == kStTrue);
                connect(edit, &QCheckBox::stateChanged, this, [=, this](int state) {
                    bool enabled = state == 2;
                    settings_->SetWebSocketEnabled(enabled);
                    edt_websocket_->setEnabled(enabled);
                });
            }
            // Streaming WebSocket port
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Streaming WebSocket Port"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edt_websocket_ = edit;
                edit->setFixedSize(input_size);
                edit->setText(std::to_string(settings_->network_listening_port_).c_str());
                edit->setEnabled(settings_->websocket_enabled_ == kStTrue);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("UdpKcp"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                cb_udp_kcp_ = edit;
                edit->setFixedSize(input_size);
                edit->setEnabled(true);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->udp_kcp_enabled_ == kStTrue);
                connect(edit, &QCheckBox::stateChanged, this, [=, this](int state) {
                    bool enabled = state == 2;
                    settings_->SetUdpKcpEnabled(enabled);
                    edt_udp_kcp_->setEnabled(enabled);
                });
            }
            // UdpKcp port
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Streaming UdpKcp Port"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edt_udp_kcp_ = edit;
                edit->setFixedSize(input_size);
                edit->setText(std::to_string(settings_->udp_listen_port_).c_str());
                edit->setEnabled(settings_->udp_kcp_enabled_ == kStTrue);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("WebRTC"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                cb_webrtc_ = edit;
                edit->setFixedSize(input_size);
                edit->setEnabled(true);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->webrtc_enabled_ == kStTrue);
                connect(edit, &QCheckBox::stateChanged, this, [=, this](int state) {
                    bool enabled = state == 2;
                    settings_->SetWebRTCEnabled(enabled);
                });
            }
            // Ethernet adapter
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Ethernet Adapter"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QComboBox(this);
                edit->setFixedSize(input_size);
                edit->addItem("Auto");
                auto all_et_info = context_->GetIps();
                int index = 0;
                int target_index_ = -1;
                for (const auto& et_info : all_et_info) {
                    if (et_info.ip_addr_ == settings_->network_listening_ip_ && !et_info.ip_addr_.empty()) {
                        target_index_ = index;
                    }
                    edit->addItem(std::format("{} {} {}", et_info.ip_addr_, (et_info.nt_type_ == IPNetworkType::kWired ? "WIRED" : "WIRELESS"), et_info.human_readable_name_).c_str());
                    index++;
                }
                if (target_index_ != -1) {
                    edit->setCurrentIndex(target_index_ + 1);
                }
                connect(edit, &QComboBox::currentIndexChanged, this, [=, this](int idx) {
                    if (idx <= 0) {
                        settings_->SetListeningIp("");
                        return;
                    }
                    auto target_ip = all_et_info.at(idx-1).ip_addr_;
                    settings_->SetListeningIp(target_ip);
                });
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            // Panel listening port
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Panel Listening Port"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edt_panel_port_ = edit;
                edit->setFixedSize(input_size);
                edit->setText(std::to_string(settings_->panel_listen_port_).c_str());
                edit->setEnabled(true);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }

            // Signaling Server
            {
                // title
                auto label = new QLabel(this);
                label->setText(tr("Signaling"));
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Server Address"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edt_sig_server_address_ = edit;
                edit->setFixedSize(input_size);
                edit->setText(settings_->sig_server_address_.c_str());
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Server Port"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edt_sig_server_port_ = edit;
                edit->setFixedSize(input_size);
                edit->setValidator(new QIntValidator);
                edit->setText(settings_->sig_server_port_.c_str());
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }

            // Coturn Address
            {
                // title
                auto label = new QLabel(this);
                label->setText(tr("Coturn"));
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Server Address"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edt_coturn_server_address_ = edit;
                edit->setFixedSize(input_size);
                edit->setText(settings_->coturn_server_address_.c_str());
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Server Port"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edt_coturn_server_port_ = edit;
                edit->setFixedSize(input_size);
                edit->setValidator(new QIntValidator);
                edit->setText(settings_->coturn_server_port_.c_str());
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            column1_layout->addLayout(segment_layout);
        }

        {
            auto func_show_err = [=](const QString& msg) {
                auto msg_box = SizedMessageBox::MakeErrorOkBox(tr("Save Settings Error"), msg);
                msg_box->exec();
            };

            auto layout = new NoMarginHLayout();
            auto btn = new QPushButton(this);
            btn->setText(tr("SAVE"));
            btn->setFixedSize(QSize(150, 35));
            btn->setStyleSheet("font-size: 14px; font-weight: 700;");
            layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                settings_->SetSigServerAddress(edt_sig_server_address_->text().toStdString());
                settings_->SetSigServerPort(edt_sig_server_port_->text().toStdString());
                settings_->SetCoturnServerAddress(edt_coturn_server_address_->text().toStdString());
                settings_->SetCoturnServerPort(edt_coturn_server_port_->text().toStdString());
                settings_->SetPanelListeningPort(edt_panel_port_->text().toInt());
                // Load again
                settings_->Load();

                this->context_->SendAppMessage(MsgSettingsChanged {
                    .settings_ = settings_,
                });

                // Save success dialog
                auto msg_box = SizedMessageBox::Make2BtnsBox(tr("Save Success"),
                                                             tr("Save settings success! Do you want to restart Renderer?"), tr("Later"), tr("Restart"));
                if (msg_box->exec() == 0) {
                    // restart server now
                    this->context_->SendAppMessage(AppMsgRestartServer{});
                }

            });

            layout->addStretch();
            column1_layout->addSpacing(30);
            column1_layout->addLayout(layout);
        }

        column1_layout->addStretch();

        setLayout(root_layout);
    }

    void StNetwork::OnTabShow() {

    }

    void StNetwork::OnTabHide() {

    }

}

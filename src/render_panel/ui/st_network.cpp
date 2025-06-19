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
#include "tc_dialog.h"
#include "tc_label.h"
#include "tc_pushbutton.h"
#include "tc_spvr_client/spvr_api.h"
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>
#include <QFileDialog>

namespace tc
{

    StNetwork::StNetwork(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent){
        auto root_layout = new NoMarginHLayout();
        auto column1_layout = new NoMarginVLayout();
        root_layout->addLayout(column1_layout);

        auto column2_layout = new NoMarginVLayout();
        root_layout->addSpacing(10);
        root_layout->addLayout(column2_layout);

        root_layout->addStretch();

        // segment encoder
        auto tips_label_width = 240;
        auto tips_label_height = 35;
        auto tips_label_size = QSize(tips_label_width, tips_label_height);
        auto input_size = QSize(280, tips_label_height);

        {
            auto segment_layout = new NoMarginVLayout();
            {
                // title
                auto label = new TcLabel(this);
                label->SetTextId("id_network_settings");
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(0);
                segment_layout->addWidget(label);
            }
            // Network type
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_websocket");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                cb_websocket_ = edit;
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->IsWebSocketEnabled());
                connect(edit, &QCheckBox::checkStateChanged, this, [=, this](Qt::CheckState state) {
                    bool enabled = state == Qt::CheckState::Checked;
                    if (!enabled) {
                        context_->PostUIDelayTask([=, this]() {
                            TcDialog dialog(tcTr("id_tips"), tcTr("id_dialog_ssl_streaming_always_on"));
                            dialog.exec();

                            settings_->SetWebSocketEnabled(true);
                            cb_websocket_->setChecked(true);
                        }, 50);
                    }
                });
            }
            // Streaming WebSocket port
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_streaming_websocket_port");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edt_websocket_ = edit;
                edit->setFixedSize(input_size);
                edit->setText(std::to_string(settings_->render_srv_port_).c_str());
                edit->setEnabled(settings_->IsWebSocketEnabled());
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_udp");
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
                auto label = new TcLabel(this);
                label->SetTextId("id_streaming_udp_port");
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
                auto label = new TcLabel(this);
                label->SetTextId("id_rtc");
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
                auto label = new TcLabel(this);
                label->SetTextId("id_ethernet_adapter");
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
                auto label = new TcLabel(this);
                label->SetTextId("id_panel_listening_port");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edt_panel_port_ = edit;
                edit->setFixedSize(input_size);
                edit->setText(std::to_string(settings_->panel_srv_port_).c_str());
                edit->setEnabled(true);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }

            // ID Server
            {
                // title
                auto label = new TcLabel(this);
                label->SetTextId("id_gr_spvr_server");
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            // Relay

            auto fn_set_spvr_input_state = [this](bool enabled) {
                if (enabled) {
                    edt_spvr_server_host_->setEnabled(true);
                    edt_spvr_server_port_->setEnabled(true);
                }
                else {
                    edt_spvr_server_host_->setEnabled(false);
                    edt_spvr_server_port_->setEnabled(false);
                }
            };

            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_relay");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setFixedSize(input_size);
                edit->setEnabled(true);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->IsRelayEnabled());
                connect(edit, &QCheckBox::checkStateChanged, this, [=, this](Qt::CheckState state) {
                    if (state == Qt::CheckState::Checked) {
                        settings_->SetRelayEnabled(true);
                    }
                    else {
                        settings_->SetRelayEnabled(false);
                    }

                    fn_set_spvr_input_state(settings_->IsRelayEnabled());
                });
            }

            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_server_host");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edt_spvr_server_host_ = edit;
                edit->setFixedSize(input_size);
                edit->setText(settings_->spvr_server_host_.c_str());
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_server_port");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edt_spvr_server_port_ = edit;
                edit->setFixedSize(input_size);
                edit->setValidator(new QIntValidator);
                edit->setText(settings_->spvr_server_port_.c_str());
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            fn_set_spvr_input_state(settings_->IsRelayEnabled());
            
            column1_layout->addLayout(segment_layout);
        }

        {
            auto layout = new NoMarginHLayout();
            auto btn = new TcPushButton(this);
            btn->SetTextId("id_save");
            btn->setFixedSize(QSize(220, 35));
            btn->setStyleSheet("font-size: 14px; font-weight: 700;");
            layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                auto spvr_host = edt_spvr_server_host_->text().toStdString();
                auto spvr_port = edt_spvr_server_port_->text().toStdString();
                settings_->SetSpvrServerHost(spvr_host);
                settings_->SetSpvrServerPort(spvr_port);
                settings_->SetPanelListeningPort(edt_panel_port_->text().toInt());

                // Load again
                settings_->Load();

                this->context_->SendAppMessage(MsgSettingsChanged {
                    .settings_ = settings_,
                });

                TcDialog dialog(tcTr("id_tips"), tcTr("id_save_settings_restart_renderer"), nullptr);
                if (dialog.exec() == kDoneOk) {
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

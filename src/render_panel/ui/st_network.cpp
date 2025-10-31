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
#include "tc_common_new/string_util.h"
#include "tc_common_new/win32/audio_device_helper.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/companion/panel_companion.h"
#include "tc_common_new/ip_util.h"
#include "tc_dialog.h"
#include "tc_label.h"
#include "tc_pushbutton.h"
#include "st_network_search.h"
#include "tc_spvr_client/spvr_api.h"
#include "tc_spvr_client/spvr_device.h"
#include "tc_relay_client/relay_api.h"
#include "tc_common_new/message_notifier.h"
#include "render_panel/spvr_scanner/spvr_scanner.h"
#include "st_network_auto_join_dialog.h"
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
        auto tips_label_width = 300;
        auto tips_label_height = 35;
        auto tips_label_size = QSize(tips_label_width, tips_label_height);
        auto input_size = QSize(280, tips_label_height);

        {
            auto segment_layout = new NoMarginVLayout();
            // Servers
            {
                // title
                auto label = new TcLabel(this);
                label->SetTextId("id_gr_spvr_server");
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(0);
                segment_layout->addWidget(label);
                segment_layout->addSpacing(2);
            }

            // Spvr access info
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_spvr_auth_access_info");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QTextEdit(this);
                edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
                edit->setLineWrapMode(QTextEdit::WidgetWidth);
                edit->setAcceptRichText(false);
                edt_spvr_access_ = edit;
                QObject::connect(edit, &QTextEdit::textChanged, this, [=, this]() {
                     auto text = edit->toPlainText();
                     auto info = this->ParseSpvrAccessInfo(text.toStdString());
                     this->DisplaySpvrAccessInfo(info);
                 });
                edit->setFixedSize(input_size.width()*2, input_size.height()*2);
                edit->setText(settings_->GetSpvrAccessInfo().c_str());
                layout->addWidget(edit);
                layout->addSpacing(15);

                {
                    auto search_layout = new NoMarginVLayout();

                    auto search = new TcPushButton();
                    search->SetTextId("id_file_trans_search");
                    search->setFixedSize(80, 32);
                    search_layout->addWidget(search);
                    connect(search, &QPushButton::clicked, this, [=, this]() {
                        this->SearchAccessInfo(false);
                    });

                    search_layout->addSpacing(5);

                    auto verify = new TcPushButton();
                    verify->SetTextId("id_verify");
                    verify->setFixedSize(80, 32);
                    search_layout->addWidget(verify);
                    connect(verify, &QPushButton::clicked, this, [=, this]() {
                        this->VerifyAccessInfo();
                    });

                    search_layout->addStretch();
                    layout->addLayout(search_layout);
                }

                layout->addStretch();

                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }

            // Manager Server
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_server_host");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setEnabled(false);
                edt_spvr_server_host_ = edit;
                edit->setFixedSize(input_size);
                edit->setText(settings_->GetSpvrServerHost().c_str());
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
                edit->setEnabled(false);
                edt_spvr_server_port_ = edit;
                edit->setFixedSize(input_size);
                edit->setValidator(new QIntValidator);
                edit->setText(std::to_string(settings_->GetSpvrServerPort()).c_str());
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }

            // Relay Server
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_relay_host");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setEnabled(false);
                edt_relay_server_host_ = edit;
                edit->setFixedSize(input_size);
                edit->setText(settings_->GetRelayServerHost().c_str());
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_relay_port");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setEnabled(false);
                edt_relay_server_port_ = edit;
                edit->setFixedSize(input_size);
                edit->setValidator(new QIntValidator);
                edit->setText(std::to_string(settings_->GetRelayServerPort()).c_str());
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }

            // PORT settings
            {
                // title
                auto label = new TcLabel(this);
                label->SetTextId("id_network_settings");
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
                segment_layout->addSpacing(2);
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
                edit->setText(std::to_string(settings_->GetRenderServerPort()).c_str());
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
                edit->setText(std::to_string(settings_->GetPanelServerPort()).c_str());
                edit->setEnabled(true);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }

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
                this->Save(false);
            });

            layout->addStretch();
            column1_layout->addSpacing(30);
            column1_layout->addLayout(layout);
        }

        column1_layout->addStretch();

        setLayout(root_layout);

        // messages
        msg_listener_ = app->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgForceClearProgramData>([=, this](const MsgForceClearProgramData& msg) {
            // clear data
            app_->GetContext()->PostUITask([=, this]() {
                edt_spvr_access_->setText("");
                edt_spvr_server_host_->setText("");
                edt_spvr_server_port_->setText("");
                edt_relay_server_host_->setText("");
                edt_relay_server_port_->setText("");
            });
        });

        //
        context_->PostUIDelayTask([=, this]() {
            this->SearchAccessInfo(true);
        }, 5000);
    }

    void StNetwork::OnTabShow() {

    }

    void StNetwork::OnTabHide() {

    }

    std::shared_ptr<SpvrAccessInfo> StNetwork::ParseSpvrAccessInfo(const std::string& info) {
        auto companion = grApp->GetCompanion();
        if (!companion) {
            return nullptr;
        }
        return companion->ParseSpvrAccessInfo(info);
    }

    void StNetwork::DisplaySpvrAccessInfo(const std::shared_ptr<SpvrAccessInfo>& info) {
        if (!info || !info->spvr_config_.IsValid() || info->relay_configs_.empty()) {
            if (edt_spvr_server_host_) {
                edt_spvr_server_host_->setText("");
            }
            if (edt_spvr_server_port_) {
                edt_spvr_server_port_->setText("");
            }
            if (edt_relay_server_host_) {
                edt_relay_server_host_->setText("");
            }
            if (edt_relay_server_port_) {
                edt_relay_server_port_->setText("");
            }
            return;
        }
        if (edt_spvr_server_host_) {
            edt_spvr_server_host_->setText(info->spvr_config_.srv_w3c_ip_.c_str());
        }
        if (edt_spvr_server_port_) {
            edt_spvr_server_port_->setText(QString::number(info->spvr_config_.srv_working_port_));
        }
        if (edt_relay_server_host_) {
            edt_relay_server_host_->setText(info->relay_configs_[0].srv_w3c_ip_.c_str());
        }
        if (edt_relay_server_port_) {
            edt_relay_server_port_->setText(QString::number(info->relay_configs_[0].srv_working_port_));
        }
    }

    void StNetwork::SaveSpvrAccessInfo() {
        auto info = edt_spvr_access_->toPlainText().toStdString();
        settings_->SetSpvrAccessInfo(info);
    }

    void StNetwork::SearchAccessInfo(bool auto_restart_render) {
        auto ac_info = app_->GetSpvrScanner()->GetSpvrAccessInfo();
        if (ac_info.empty()) {
            return;
        }
        if (ac_info.size() == 1) {
            std::shared_ptr<StNetworkSpvrAccessInfo> info = nullptr;
            for (const auto& [k, v] : ac_info) {
                info = v;
            }
            if (info) {
                if (settings_->GetSpvrServerHost() != info->spvr_ip_ || settings_->GetSpvrServerPort() != info->spvr_port_
                    || settings_->GetRelayServerHost() != info->relay_ip_ || settings_->GetRelayServerPort() != info->relay_port_ || !auto_restart_render) {
                    StNetworkAutoJoinDialog dialog(app_, info);
                    if (dialog.exec() == 0) {
                        edt_spvr_access_->setText(info->origin_info_.c_str());
                        if (auto_restart_render) {
                            this->Save(auto_restart_render);
                        }
                    }
                }
            }
        }
        else {
            StNetworkSearch nt_search(app_, this);
            if (nt_search.exec() == 0) {
                auto selected_item = nt_search.GetSelectedItem();
                if (!selected_item) {
                    LOGE("Not a valid spvr item !");
                    return;
                }
                if (selected_item->spvr_ip_.empty() || selected_item->relay_ip_.empty()) {
                    TcDialog dialog(tcTr("id_error"), tcTr("id_verify_spvr_failed"));
                    dialog.exec();
                    return;
                }
                edt_spvr_access_->setText(selected_item->origin_info_.c_str());
            }
        }
    }

    void StNetwork::VerifyAccessInfo() {
        // 1. verify spvr server
        auto ac_info = ParseSpvrAccessInfo(edt_spvr_access_->toPlainText().toStdString());
        if (!ac_info) {
            LOGE("Parse access info failed: {}", edt_spvr_access_->toPlainText().toStdString());
            return;
        }

        {
            auto appkey = ac_info->spvr_config_.srv_appkey_;
            auto r = spvr::SpvrApi::Ping(ac_info->spvr_config_.srv_w3c_ip_, ac_info->spvr_config_.srv_working_port_, appkey);
            if (!r.has_value() || !r.value()) {
                TcDialog dialog(tcTr("id_error"), tcTr("id_verify_spvr_failed"));
                dialog.exec();
                return;
            }
        }

        // 2. verify relay server
        {
            if (ac_info->relay_configs_.empty()) {
                TcDialog dialog(tcTr("id_error"), tcTr("id_verify_relay_failed"));
                dialog.exec();
                return;
            }
            for (const auto& cfg : ac_info->relay_configs_) {
                auto r = relay::RelayApi::Ping(cfg.srv_w3c_ip_, cfg.srv_working_port_, cfg.srv_appkey_);
                if (!r.has_value() || !r.value()) {
                    TcDialog dialog(tcTr("id_error"), tcTr("id_verify_relay_failed"));
                    dialog.exec();
                    return;
                }
            }
        }

        TcDialog dialog(tcTr("id_tips"), tcTr("id_verify_success"));
        dialog.exec();
    }

    void StNetwork::Save(bool auto_restart_render) {
        auto spvr_host = edt_spvr_server_host_->text().toStdString();
        auto spvr_port = edt_spvr_server_port_->text().toStdString();
        auto relay_host = edt_relay_server_host_->text().toStdString();
        auto relay_port = edt_relay_server_port_->text().toStdString();
        bool force_update_device_id = false;
        if (!spvr_host.empty()
            && (settings_->GetSpvrServerHost() != spvr_host || settings_->GetSpvrServerPort() != std::atoi(spvr_port.c_str()))) {
            force_update_device_id = true;
            settings_->SetDeviceId("");
            settings_->SetDeviceRandomPwd("");
            LOGW("Clear old device id, force updating device id.");
        }
        settings_->SetSpvrServerHost(spvr_host);
        settings_->SetSpvrServerPort(spvr_port);
        settings_->SetPanelServerPort(edt_panel_port_->text().toInt());

        settings_->SetRelayServerHost(relay_host);
        settings_->SetRelayServerPort(relay_port);

        SaveSpvrAccessInfo();

        // Load again
        settings_->Load();

        // companion
        auto companion = grApp->GetCompanion();
        if (companion) {
            companion->UpdateSpvrServerConfig(settings_->GetSpvrServerHost(), settings_->GetSpvrServerPort());
            auto auth = companion->RequestAuth();
            if (!auth) {
                TcDialog dialog(tcTr("id_warning"), tcTr("id_cant_request_auth"), nullptr);
                dialog.exec();
                return;
            }
            LOGI("Requested auth, id: {} , appkey: {}", auth->auth_id_, auth->appkey_);
        }

        // refresh settings
        grApp->RefreshClientManagerSettings();

        // request id if needed
        auto device_id = settings_->GetDeviceId();
        // request function
        auto fn_request_new_device_id = [=, this]() -> bool {
            if (!grApp->RequestNewClientId(true, true)) {
                LOGE("Request Device ID failed!");
                TcDialog dialog(tcTr("id_warning"), tcTr("id_request_device_id_failed"), nullptr);
                dialog.exec();
                return false;
            }
            else {
                LOGI("Request Device ID success!");
                return true;
            }
        };

        // request a new device id
        if (device_id.empty()) {
            if (!fn_request_new_device_id()) {
                return;
            }
        }
        else {
            // check the device id is valid or not
            auto r = spvr::SpvrApi::QueryDevice(settings_->GetSpvrServerHost(), settings_->GetSpvrServerPort(), grApp->GetAppkey(), device_id);
            if (!r.has_value() || r.value()->device_id_.empty()) {
                // request a new one
                LOGI("Can't query the device id : {} in server, will request a new one.", settings_->GetDeviceId());
                if (!fn_request_new_device_id()) {
                    return;
                }
            }
        }

        this->context_->SendAppMessage(MsgSettingsChanged {
            .settings_ = settings_,
            .force_update_device_id_ = force_update_device_id,
        });

        if (auto_restart_render) {
            this->context_->SendAppMessage(AppMsgRestartServer{});
        }
        else {
            TcDialog dialog(tcTr("id_tips"), tcTr("id_save_settings_restart_renderer"), nullptr);
            if (dialog.exec() == kDoneOk) {
                this->context_->SendAppMessage(AppMsgRestartServer{});
            }
        }

    }

}

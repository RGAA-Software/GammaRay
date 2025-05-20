//
// Created by RGAA on 2024-04-11.
//

#include "st_general.h"
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
#include "tc_qt_widget/tc_label.h"
#include "tc_dialog.h"
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

    StGeneral::StGeneral(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
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
                auto label = new TcLabel(this);
                label->SetTextId("id_appearance");
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            // language
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_language");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QComboBox(this);
                edit->addItem(QStringLiteral("English"));
                edit->addItem(QStringLiteral("简体中文"));
                edit->addItem(QStringLiteral("繁体中文"));

                auto kind = tcTrMgr()->GetSelectedLanguage();
                if (kind == LanguageKind::kSimpleCN) {
                    edit->setCurrentIndex(1);
                }
                else if (kind == LanguageKind::kTraditionalCN) {
                    edit->setCurrentIndex(2);
                }

                cb_language_ = edit;
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(10);
                segment_layout->addLayout(layout);

                connect(edit, &QComboBox::currentIndexChanged, this, [=](int index) {
                    if (index == 0) {
                        tcTrMgr()->LoadLanguage(LanguageKind::kEnglish);
                    }
                    else if (index == 1) {
                        tcTrMgr()->LoadLanguage(LanguageKind::kSimpleCN);
                    }
                    else if (index == 2) {
                        tcTrMgr()->LoadLanguage(LanguageKind::kTraditionalCN);
                    }
                    tcTrMgr()->Translate();
                });
            }

            column1_layout->addLayout(segment_layout);
        }

        {
            auto segment_layout = new NoMarginVLayout();
            {
                // title
                auto label = new TcLabel(this);
                label->SetTextId("id_encoder_settings");
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            // Bitrate
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_bitrate");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setValidator(new QIntValidator(0, 1000));
                et_bitrate_ = edit;
                et_bitrate_->setText(settings_->encoder_bitrate_.c_str());
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(10);
                segment_layout->addLayout(layout);
            }


            // FPS
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_fps");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setValidator(new QIntValidator(0, 1000));
                et_fps_ = edit;
                et_fps_->setText(settings_->encoder_fps_.c_str());
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(10);
                segment_layout->addLayout(layout);
            }

            // Format
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_format");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QComboBox(this);
                edit->setFixedSize(input_size);
                edit->addItem("H264");
                edit->addItem("H265(HEVC)");
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);

                auto is_h265 = settings_->encoder_format_ == "h265";
                edit->setCurrentIndex(is_h265 ? 1 : 0);

                connect(edit, &QComboBox::currentIndexChanged, this, [=, this](int idx) {
                    settings_->SetEncoderFormat(idx);
                });
            }
            // Resize resolution
            auto func_set_res_edit_enabled = [=, this](bool enabled) {
                if (enabled) {
                    et_res_width_->setEnabled(true);
                    et_res_height_->setEnabled(true);
                } else {
                    et_res_width_->setEnabled(false);
                    et_res_height_->setEnabled(false);
                }
            };
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_resize_resolution");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setFixedSize(input_size);
                cb_resize_res_ = edit;
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(!settings_->IsEncoderResTypeOrigin());
                connect(edit, &QCheckBox::stateChanged, this, [=, this](int state) {
                    bool enabled = state == 2;
                    func_set_res_edit_enabled(enabled);
                    settings_->SetEnableResResize(enabled);
                });
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_resize_width");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setValidator(new QIntValidator(0, 80000));
                et_res_width_ = edit;
                et_res_width_->setText(settings_->encoder_width_.c_str());
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_resize_height");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setValidator(new QIntValidator(0, 80000));
                et_res_height_ = edit;
                et_res_height_->setText(settings_->encoder_height_.c_str());
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }

            func_set_res_edit_enabled(!settings_->IsEncoderResTypeOrigin());

            column1_layout->addLayout(segment_layout);
        }
        {
            auto segment_layout = new NoMarginVLayout();
            {
                // title
                auto label = new TcLabel(this);
                label->SetTextId("id_capture_settings");
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            // capture video
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_capture_video");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);

                edit->setChecked(settings_->capture_video_ == kStTrue);
                connect(edit, &QCheckBox::stateChanged, this, [=, this](int state) {
                    bool enabled = state == 2;
                    settings_->SetCaptureVideo(enabled);
                    cb_capture_monitor_->setEnabled(enabled);
                });
            }
            // Capture monitor
#if 0
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Capture Video Monitor"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QComboBox(this);
                cb_capture_monitor_ = edit;
                edit->setFixedSize(input_size);
                edit->setEnabled(settings_->capture_video_ == kStTrue);
                auto adapters = DxgiMonitorDetector::Instance()->GetAdapters();
                int idx = 0;
                int target_idx = -1;
                for (const auto& adapter : adapters) {
                    edit->addItem(std::format("{} [{}x{}]", adapter.display_name, adapter.width, adapter.height).c_str());
                    LOGI("capture: {}, display: {}", settings_->capture_monitor_, adapter.display_name);
                    if (settings_->capture_monitor_ == adapter.display_name) {
                        target_idx = idx;
                    }
                    idx++;
                }

                if (target_idx != -1) {
                    edit->setCurrentIndex(target_idx);
                    settings_->SetCaptureMonitor(adapters.at(target_idx).display_name);
                    LOGI("capture target index: {}", target_idx);
                } else {
                    settings_->SetCaptureMonitor("");
                }
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);

                connect(edit, &QComboBox::currentIndexChanged, this, [=, this](int idx) {
                    auto monitor = adapters.at(idx).display_name;
                    settings_->SetCaptureMonitor(monitor);
                });

            }
#endif
            // capture audio
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_capture_audio");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->capture_audio_ == kStTrue);
                connect(edit, &QCheckBox::stateChanged, this, [=, this](int state) {
                    bool enabled = state == 2;
                    settings_->SetCaptureAudio(enabled);
                    cb_capture_audio_device_name_->setEnabled(enabled);
                });
            }

            // Capture Audio
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_capture_audio_device");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QComboBox(this);
                cb_capture_audio_device_name_ = edit;
                edit->setEnabled(settings_->capture_audio_ == kStTrue);
                edit->setFixedSize(input_size);

                auto devices = AudioDeviceHelper::DetectAudioDevices();

                int idx = 0;
                int target_idx = -1;
                int default_idx = -1;
                for (const auto& device : devices) {
                    edit->addItem(device.name_.c_str());
                    if (settings_->capture_audio_device_ == device.id_) {
                        target_idx = idx;
                    }
                    if (device.default_device_) {
                        default_idx = idx;
                    }
                    idx++;
                }
                if (target_idx != -1) {
                    edit->setCurrentIndex(target_idx);
                } else {
                    if (default_idx != -1) {
                        edit->setCurrentIndex(default_idx);
                    }
                    settings_->SetCaptureAudioDeviceId("");
                }
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);

                connect(edit, &QComboBox::currentIndexChanged, this, [=, this](int idx) {
                    auto target_device_id = devices.at(idx).id_;
                    settings_->SetCaptureAudioDeviceId(target_device_id);
                });
            }
            column1_layout->addLayout(segment_layout);
        }

        // moved to Network page
        if (false) {
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
                edit->setFixedSize(input_size);
                edit->setEnabled(true);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->websocket_enabled_ == kStTrue);
                connect(edit, &QCheckBox::stateChanged, this, [=, this](int state) {
                    bool enabled = state == 2;
                    //settings_->SetCaptureAudio(enabled);
                });
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("WebRTC"));
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
                edit->setChecked(settings_->webrtc_enabled_ == kStTrue);
                connect(edit, &QCheckBox::stateChanged, this, [=, this](int state) {
                    bool enabled = state == 2;
                    //settings_->SetCaptureAudio(enabled);
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
            // Http port
            if (0) {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Http Port"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setFixedSize(input_size);
                edit->setText(std::to_string(settings_->http_server_port_).c_str());
                edit->setEnabled(false);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            // Websocket port
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Panel Listening Port"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setFixedSize(input_size);
                edit->setText(std::to_string(settings_->panel_srv_port_).c_str());
                edit->setEnabled(false);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            // Streaming port
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Renderer Streaming Port"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setFixedSize(input_size);
                edit->setText(std::to_string(settings_->render_srv_port_).c_str());
                edit->setEnabled(false);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            column1_layout->addLayout(segment_layout);
        }

        {
            auto func_show_err = [=](const QString& msg) {
//                auto msg_box = SizedMessageBox::MakeErrorOkBox(tr("Save Settings Error"), msg);
//                msg_box->exec();

                TcDialog dialog(tr("Error"), msg, nullptr);
                dialog.exec();
            };

            auto layout = new NoMarginHLayout();
            auto btn = new QPushButton(this);
            btn->setText(tr("SAVE"));
            btn->setFixedSize(QSize(150, 35));
            btn->setStyleSheet("font-size: 14px; font-weight: 700;");
            layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                // bitrate
                auto bitrate = et_bitrate_->text().toInt();
                if (bitrate == 0) {
                    func_show_err(tr("Bitrate is none!"));
                    return;
                }
                settings_->SetBitrate(bitrate);

                // fps
                auto fps = et_fps_->text().toInt();
                if (fps < 15 || fps > 120) {
                    func_show_err(tr("FPS between 15 and 120!"));
                    return;
                }
                settings_->SetFPS(fps);

                if (cb_resize_res_->isChecked()) {
                    // resize width
                    auto res_width = et_res_width_->text().toInt();
                    if (res_width == 0) {
                        func_show_err(tr("Resolution width is none!"));
                        return;
                    }
                    settings_->SetResWidth(res_width);

                    // resize height
                    auto res_height = et_res_height_->text().toInt();
                    if (res_height == 0) {
                        func_show_err(tr("Resolution height is none!"));
                        return;
                    }
                    settings_->SetResHeight(res_height);
                }

#if 0
                auto monitor_name = cb_capture_monitor_->currentText();
                auto adapters = DxgiMonitorDetector::Instance()->GetAdapters();
                for (const auto& adapter : adapters) {
                    if (monitor_name.contains(QString::fromStdString(adapter.display_name))) {
                        settings_->SetCaptureMonitor(adapter.display_name);
                    }
                }
#endif

                auto audio_devices = AudioDeviceHelper::DetectAudioDevices();
                auto audio_device_name = cb_capture_audio_device_name_->currentText().toStdString();
                for (const auto& dev : audio_devices) {
                    if (dev.name_ == audio_device_name) {
                        settings_->SetCaptureAudioDeviceId(dev.id_);
                    }
                }

                // Load again
                settings_->Load();

                // Save success dialog
//                auto msg_box = SizedMessageBox::Make2BtnsBox(tr("Save Success"),
//                    tr("Save settings success! Do you want to restart Renderer?"), tr("Later"), tr("Restart"));
//                if (msg_box->exec() == 0) {
//                    // restart server now
//                    this->context_->SendAppMessage(AppMsgRestartServer{});
//                }

                TcDialog dialog(tr("Tips"), tr("Save settings success! Do you want to restart Renderer?"), nullptr);
                if (dialog.exec() == kDoneOk) {
                    this->context_->SendAppMessage(AppMsgRestartServer{});
                }

            });

            layout->addStretch();
            column1_layout->addSpacing(30);
            column1_layout->addLayout(layout);
        }
        column1_layout->addStretch();

        // column 2
        {
            auto segment_layout = new NoMarginVLayout();
            {
                // title
                auto label = new QLabel(this);
                label->setText(tr("File Transfer"));
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            // default transfer folder
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Drag&Drop Receive Folder"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setFixedSize(QSize(360, 35));
                edit->setText(settings_->file_transfer_folder_.c_str());
                layout->addWidget(edit);

                auto btn = new QPushButton(this);
                btn->setFixedSize(120, 35);
                btn->setText("Select");
                connect(btn, &QPushButton::clicked, this, [=]() {
                    QString filename = QFileDialog::getExistingDirectory();
                    if (filename.isEmpty()) {
                        return;
                    }
                    edit->setText(filename);
                    settings_->SetFileTransferFolder(filename.toStdString());
                });
                layout->addSpacing(20);
                layout->addWidget(btn);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);

            }
            {

            }
            column2_layout->addLayout(segment_layout);

            column2_layout->addStretch();
        }

        setLayout(root_layout);

    }

    void StGeneral::OnTabShow() {

    }

    void StGeneral::OnTabHide() {

    }

}
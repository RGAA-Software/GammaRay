//
// Created by RGAA on 2024-04-11.
//

#include "st_general.h"
#include "widgets/no_margin_layout.h"
#include "gr_context.h"
#include "gr_application.h"
#include "gr_settings.h"
#include "widgets/sized_msg_box.h"
#include "util/dxgi_mon_detector.h"
#include "tc_common_new/log.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>
#include <QAudioDevice>
#include <QMediaDevices>

namespace tc
{

    StGeneral::StGeneral(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginVLayout();
        // segment encoder
        auto tips_label_width = 220;
        auto tips_label_height = 35;
        auto tips_label_size = QSize(tips_label_width, tips_label_height);
        auto input_size = QSize(240, tips_label_height-5);

        {
            auto segment_layout = new NoMarginVLayout();
            {
                // title
                auto label = new QLabel(this);
                label->setText(tr("Encoder Settings"));
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            // Bitrate
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Bitrate(M)"));
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
            // Format
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Format"));
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
                auto label = new QLabel(this);
                label->setText(tr("Resize Resolution"));
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
                auto label = new QLabel(this);
                label->setText(tr("Resize Width"));
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
                auto label = new QLabel(this);
                label->setText(tr("Resize Height"));
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

            root_layout->addLayout(segment_layout);
        }
        {
            auto segment_layout = new NoMarginVLayout();
            {
                // title
                auto label = new QLabel(this);
                label->setText(tr("Capture Settings"));
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            // capture video
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Capture Video"));
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
                });
            }
            // Capture monitor
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Capture Video Monitor"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QComboBox(this);
                edit->setFixedSize(input_size);
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
            // capture audio
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Capture Audio"));
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
                });
            }

            // Capture Audio
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Capture Audio Device"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QComboBox(this);
                edit->setFixedSize(input_size);
                const QList<QAudioDevice> devices = QMediaDevices::audioOutputs();
                int idx = 0;
                int target_idx = -1;
                for (const QAudioDevice &device : devices) {
                    qDebug() << "音频输出设备: " << device.description().toStdString()  << ", " << device.id();
                    edit->addItem(device.description());
                    if (settings_->capture_audio_device_ == device.id().toStdString()) {
                        target_idx = idx;
                    }
                    idx++;
                }
                if (target_idx != -1) {
                    edit->setCurrentIndex(target_idx);
                } else {
                    settings_->SetCaptureAudioDevice("");
                }
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);

                connect(edit, &QComboBox::currentIndexChanged, this, [=, this](int idx) {
                    auto target_device_id = devices.at(idx).id().toStdString();
                    settings_->SetCaptureAudioDevice(target_device_id);
                });
            }
            root_layout->addLayout(segment_layout);
        }

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
                label->setText(tr("Network Type"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QComboBox(this);
                edit->setFixedSize(input_size);
                edit->addItem("Websocket");
                //edit->addItem("WebRTC");
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            // Http port
            {
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
                label->setText(tr("Websocket Port"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setFixedSize(input_size);
                edit->setText(std::to_string(settings_->ws_server_port_).c_str());
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
                label->setText(tr("Streaming Port"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                edit->setFixedSize(input_size);
                edit->setText(std::to_string(settings_->network_listen_port_).c_str());
                edit->setEnabled(false);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }
            root_layout->addLayout(segment_layout);
        }

        {
            auto func_show_err = [=](const QString& msg) {
                auto msg_box = SizedMessageBox::MakeErrorOkBox(tr("Save Settings Error"), msg);
                msg_box->exec();
            };

            auto layout = new NoMarginHLayout();
            auto btn = new QPushButton(this);
            btn->setText(tr("S.A.V.E"));
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

                // Load again
                settings_->Load();

                // Save success dialog
                auto msg_box = SizedMessageBox::MakeOkBox(tr("Save Success"), tr("Save settings success !"));
                msg_box->exec();
            });

            layout->addStretch();
            root_layout->addSpacing(30);
            root_layout->addLayout(layout);
        }

        root_layout->addStretch();
        setLayout(root_layout);
    }

    void StGeneral::OnTabShow() {

    }

    void StGeneral::OnTabHide() {

    }

}
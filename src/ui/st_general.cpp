//
// Created by RGAA on 2024-04-11.
//

#include "st_general.h"
#include "widgets/no_margin_layout.h"
#include "gr_context.h"
#include "gr_application.h"
#include "gr_settings.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>

namespace tc
{

    StGeneral::StGeneral(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginVLayout();
        // segment encoder
        auto tips_label_width = 220;
        auto tips_label_height = 35;
        auto tips_label_size = QSize(tips_label_width, tips_label_height);
        auto input_size = QSize(120, tips_label_height-5);

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
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->enable_res_resize_);
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
                et_res_width_ = edit;
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
                et_res_height_ = edit;
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
            }

            func_set_res_edit_enabled(settings_->enable_res_resize_);

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
            auto layout = new NoMarginHLayout();
            auto label = new QPushButton(this);
            label->setText(tr("SAVE"));
            label->setFixedSize(QSize(120, 35));
            label->setStyleSheet("font-size: 14px; font-weight: 700;");
            layout->addWidget(label);

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
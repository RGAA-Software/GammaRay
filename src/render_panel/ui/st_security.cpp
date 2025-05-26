//
// Created by RGAA on 2024-06-10.
//

#include "st_security.h"
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
#include "tc_spvr_client/spvr_manager.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>
#include <QFileDialog>

namespace tc
{

    StSecurity::StSecurity(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent){
        auto root_layout = new NoMarginHLayout();
        auto column1_layout = new NoMarginVLayout();
        root_layout->addLayout(column1_layout);

        auto column2_layout = new NoMarginVLayout();
        root_layout->addSpacing(10);
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
                label->setText(tr("Security Settings"));
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(0);
                segment_layout->addWidget(label);
            }
            // Network type
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Allowed Mouse&Keyboard"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                //cb_websocket_ = edit;
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
                    //edt_websocket_->setEnabled(enabled);
                });
            }

            // ID Server
            {
                // title
                auto label = new QLabel(this);
                label->setText(tr("GammaRay Server"));
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            {
                auto layout = new NoMarginHLayout();
                auto label = new QLabel(this);
                label->setText(tr("Server Host"));
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                //edt_spvr_server_host_ = edit;
                edit->setFixedSize(input_size);
                edit->setText(settings_->spvr_server_host_.c_str());
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
                //edt_spvr_server_port_ = edit;
                edit->setFixedSize(input_size);
                edit->setValidator(new QIntValidator);
                edit->setText(settings_->spvr_server_port_.c_str());
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

//                TcDialog dialog(tr("Tips"), tr("Save settings success! Do you want to restart Renderer?"), nullptr);
//                if (dialog.exec() == kDoneOk) {
//                    this->context_->SendAppMessage(AppMsgRestartServer{});
//                }

            });

            layout->addStretch();
            column1_layout->addSpacing(30);
            column1_layout->addLayout(layout);
        }

        column1_layout->addStretch();

        setLayout(root_layout);
    }

    void StSecurity::OnTabShow() {

    }

    void StSecurity::OnTabHide() {

    }

}

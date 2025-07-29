//
// Created by RGAA on 2024-06-10.
//

#include "st_security.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>
#include <QFileDialog>
#include "tc_dialog.h"
#include "tc_label.h"
#include "tc_pushbutton.h"
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
#include "tc_common_new/ip_util.h"
#include "tc_spvr_client/spvr_api.h"
#include "input_safety_pwd_dialog.h"

namespace tc
{

    StSecurity::StSecurity(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent) {
        settings_ = GrSettings::Instance();
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
            {
                // title
                auto label = new TcLabel(this);
                label->SetTextId("id_security_settings");
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(0);
                segment_layout->addWidget(label);
            }

            // Security password
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_long_term_password");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new TcPushButton(this);
                edit->SetTextId("id_set");
                edit->setFixedSize(QSize(80, 30));
                edit->setEnabled(true);
                layout->addWidget(edit, 0, Qt::AlignVCenter);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                connect(edit, &QPushButton::clicked, this, [=, this]() {
                    InputSafetyPwdDialog dialog(app_, this);
                    dialog.exec();
                });
            }

            // Mouse&Keyboard
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_allowed_mouse_keyboard");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->IsBeingOperatedEnabled());
                connect(edit, &QCheckBox::checkStateChanged, this, [=, this](Qt::CheckState state) {
                    settings_->SetCanBeOperated(state == Qt::CheckState::Checked);
                });
            }

            // File Transfer
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_allowed_file_transfer");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->IsFileTransferEnabled());
                connect(edit, &QCheckBox::checkStateChanged, this, [=, this](Qt::CheckState state) {
                    settings_->SetFileTransferEnabled(state == Qt::CheckState::Checked);
                });
            }

            // SSL Always Enabled
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_ssl_enabled");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->IsSSLConnectionEnabled());

                connect(edit, &QCheckBox::stateChanged, this, [=, this](int state) {
                    bool enabled = state == 2;
                    if (!enabled) {
                        context_->PostUIDelayTask([=, this]() {
                            TcDialog dialog(tcTr("id_tips"), tcTr("id_dialog_ssl_always_on"));
                            dialog.exec();
                            edit->setChecked(settings_->IsSSLConnectionEnabled());
                        }, 50);
                    }
                });
            }

            // record visitor
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_record_visitor");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->IsVisitHistoryEnabled());

                connect(edit, &QCheckBox::stateChanged, this, [=, this](int state) {
                    bool enabled = state == 2;
                    if (!enabled) {
                        context_->PostUIDelayTask([=, this]() {
                            TcDialog dialog(tcTr("id_tips"), tcTr("id_dialog_record_visitor_always_on"));
                            dialog.exec();
                            edit->setChecked(settings_->IsVisitHistoryEnabled());
                        }, 50);
                    }
                });
            }

            // record file transfer
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_record_file_transfer");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->IsFileTransferHistoryEnabled());

                connect(edit, &QCheckBox::stateChanged, this, [=, this](int state) {
                    bool enabled = state == 2;
                    if (!enabled) {
                        context_->PostUIDelayTask([=, this]() {
                            TcDialog dialog(tcTr("id_tips"), tcTr("id_dialog_record_file_transfer_always_on"));
                            dialog.exec();
                            edit->setChecked(settings_->IsFileTransferHistoryEnabled());
                        }, 50);
                    }
                });
            }

            // disconnected auto lock screen
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_disconnect_auto_lock_screen");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->IsDisconnectAutoLockScreenEnabled());

                connect(edit, &QCheckBox::checkStateChanged, this, [=, this](Qt::CheckState state) {
                    settings_->SetDisconnectAutoLockScreen(state == Qt::CheckState::Checked);
                });
            }

            // clear all data
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_clear_data");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new TcPushButton(this);
                edit->setProperty("class", "danger");
                edit->SetTextId("id_clear");
                edit->setFixedSize(QSize(80, 30));
                edit->setEnabled(true);
                layout->addWidget(edit, 0, Qt::AlignVCenter);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                connect(edit, &QPushButton::clicked, this, [=, this]() {
                    TcDialog dialog(tcTr("id_clear"), tcTr("id_ask_clear_data"), this);
                    if (dialog.exec() == kDoneOk) {
                        // clear
                        settings_->ClearData();
                        context_->SendAppMessage(MsgForceClearProgramData{});
                    }
                });
            }

            ///
            {
                // title
                auto label = new TcLabel(this);
                label->SetTextId("id_application");
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }

            // develop mode
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_developer_mode");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setFixedSize(input_size);
                layout->addWidget(edit);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                edit->setChecked(settings_->IsDevelopMode());
                connect(edit, &QCheckBox::checkStateChanged, this, [=, this](Qt::CheckState state) {
                    auto enabled = state == Qt::CheckState::Checked;
                    settings_->SetDevelopModeEnabled(enabled);
                    context_->SendAppMessage(MsgDevelopModeUpdated {
                        .enabled_ = enabled,
                    });
                });
            }

            // Exit All Programs
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_exit_all_programs");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new TcPushButton(this);
                edit->setProperty("class", "danger");
                edit->SetTextId("id_exit");
                edit->setFixedSize(QSize(80, 30));
                edit->setEnabled(true);
                layout->addWidget(edit, 0, Qt::AlignVCenter);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                connect(edit, &QPushButton::clicked, this, [=, this]() {
                    context_->SendAppMessage(MsgForceStopAllPrograms{
                        .uninstall_service_ = false,
                    });
                });
            }

            // Uninstall Programs
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_uninstall_all_programs");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new TcPushButton(this);
                edit->setProperty("class", "danger");
                edit->SetTextId("id_uninstall");
                edit->setFixedSize(QSize(80, 30));
                edit->setEnabled(true);
                layout->addWidget(edit, 0, Qt::AlignVCenter);
                layout->addStretch();
                segment_layout->addSpacing(5);
                segment_layout->addLayout(layout);
                connect(edit, &QPushButton::clicked, this, [=, this]() {
                    context_->SendAppMessage(MsgForceStopAllPrograms{
                        .uninstall_service_ = true,
                    });
                });
            }

            column1_layout->addLayout(segment_layout);
        }
        column1_layout->addStretch();

        setLayout(root_layout);
    }

    void StSecurity::OnTabShow() {

    }

    void StSecurity::OnTabHide() {

    }

}

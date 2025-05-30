//
// Created by RGAA on 2023-08-18.
//

#include "input_safety_pwd_dialog.h"
#include <QValidator>
#include <QButtonGroup>
#include <QRadioButton>
#include <QTextEdit>
#include "tc_dialog.h"
#include "render_panel/database/stream_item.h"
#include "client/ct_app_message.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/gr_settings.h"
#include "tc_common_new/log.h"
#include "tc_common_new/http_client.h"
#include "tc_qt_widget/tc_password_input.h"
#include "tc_manager_client/mgr_client_sdk.h"
#include "tc_manager_client/mgr_device_operator.h"
#include "tc_manager_client/mgr_device.h"

namespace tc
{

    InputSafetyPwdDialog::InputSafetyPwdDialog(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        app_ = app;
        context_ = app_->GetContext();
        setFixedSize(375, 300);
        CreateLayout();
    }

    InputSafetyPwdDialog::~InputSafetyPwdDialog() = default;

    void InputSafetyPwdDialog::CreateLayout() {
        setWindowTitle(tr("Input Security password"));
        auto settings = GrSettings::Instance();

        auto item_width = 320;
        auto edit_size = QSize(item_width, 35);

        auto root_layout = new NoMarginHLayout();
        auto content_layout = new NoMarginVLayout();
        root_layout->addStretch();
        root_layout->addLayout(content_layout);
        root_layout->addStretch();
        root_layout_->addLayout(root_layout);

        content_layout->addSpacing(25);

        // password
        {
            auto layout = new NoMarginVLayout();

            auto label = new QLabel(this);
            label->setFixedWidth(item_width);
            label->setText("Password");
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            pwd_input_ = new TcPasswordInput(this);
            pwd_input_->setFixedSize(edit_size);
            pwd_input_->SetPassword(settings->device_safety_pwd_.c_str());

            layout->addWidget(pwd_input_);
            layout->addStretch();

            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(25);

        // password again
        {
            auto layout = new NoMarginVLayout();

            auto label = new QLabel(this);
            label->setFixedWidth(item_width);
            label->setText("Password Again");
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            pwd_input_again_ = new TcPasswordInput(this);
            pwd_input_again_->setFixedSize(edit_size);
            pwd_input_again_->SetPassword(settings->device_safety_pwd_.c_str());
            layout->addWidget(pwd_input_again_);
            layout->addStretch();

            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(50);

        // sure button
        {
            auto layout = new NoMarginVLayout();
            auto btn_sure = new QPushButton(tr("OK"));
            connect(btn_sure, &QPushButton::clicked, this, [=, this] () {
                if (settings->device_id_.empty()) {
                    TcDialog warn_dialog("Warn", "Your device is NOT managed, your device id is empty.", this);
                    warn_dialog.exec();
                    return;
                }

                auto pwd = pwd_input_->GetPassword();
                auto pwd_again = pwd_input_again_->GetPassword();
                if (pwd.isEmpty() || (pwd != pwd_again)) {
                    TcDialog warn_dialog("Warn", "Password is invalid, please re-input.", this);
                    warn_dialog.exec();
                    return;
                }

                // update safety pwd
                auto dev_opt = app_->GetDeviceOperator();
                auto device = dev_opt->UpdateSafetyPwd(settings->device_id_, pwd.toStdString());
                if (device && device->safety_pwd_ == pwd.toStdString()) {
                    settings->SetDeviceSafetyPwd(device->safety_pwd_);
                    context_->NotifyAppMessage("Update Security Success", "The new security password has updated.");
                    done(0);
                }
                else {
                    TcDialog warn_dialog("Warn", "Update security password failed, please try again.", this);
                    warn_dialog.exec();
                }
            });

            layout->addWidget(btn_sure);
            btn_sure->setFixedSize(QSize(item_width, 35));
            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(10);
        root_layout_->addStretch();
    }

    void InputSafetyPwdDialog::paintEvent(QPaintEvent *event) {
        TcCustomTitleBarDialog::paintEvent(event);
    }

    QString InputSafetyPwdDialog::GetInputPassword() {
        return pwd_input_->GetPassword();
    }

    void InputSafetyPwdDialog::closeEvent(QCloseEvent* ) {
        done(1);
    }

}
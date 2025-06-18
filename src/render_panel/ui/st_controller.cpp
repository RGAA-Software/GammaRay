//
// Created by RGAA on 2024-06-10.
//

#include "st_controller.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths>

#include "tc_label.h"
#include "tc_dialog.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_qt_widget/tc_pushbutton.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_settings.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/win32/dxgi_mon_detector.h"
#include "tc_common_new/win32/audio_device_helper.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/gr_settings.h"
#include "tc_common_new/ip_util.h"
#include "tc_spvr_client/spvr_api.h"
#include "input_safety_pwd_dialog.h"

namespace tc
{

    StController::StController(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent){
        settings_ = GrSettings::Instance();
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

        // 通用设置
        {
            auto segment_layout = new NoMarginVLayout();
            {
                // title
                auto label = new TcLabel(this);
                label->SetTextId("id_general_settings");
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(0);
                segment_layout->addWidget(label);
            }

            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_start_max_window");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QCheckBox(this);
                edit->setChecked(settings_->IsMaxWindowEnabled());
                layout->addWidget(edit);
                connect(edit, &QCheckBox::checkStateChanged, this, [=, this](Qt::CheckState state) {
                    if (state == Qt::CheckState::Checked) {
                        settings_->SetShowingMaxWindow(true);
                    }
                    else {
                        settings_->SetShowingMaxWindow(false);
                    }
                });

                layout->addStretch();
                segment_layout->addSpacing(10);
                segment_layout->addLayout(layout);
            }
            column1_layout->addLayout(segment_layout);
        }

        // screen recording 录屏设置
        {
            auto segment_layout = new NoMarginVLayout();
            {
                // title
                auto label = new TcLabel(this);
                label->SetTextId("id_screen_recording_settings");
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(20);
                segment_layout->addWidget(label);
            }
            // save path
            {
                auto layout = new NoMarginHLayout();
                auto label = new TcLabel(this);
                label->SetTextId("id_screen_recording_path");
                label->setFixedSize(tips_label_size);
                label->setStyleSheet("font-size: 14px; font-weight: 500;");
                layout->addWidget(label);

                auto edit = new QLineEdit(this);
                et_screen_recording_path_ = edit;
                auto size = input_size;
                size.setWidth(size.width());
                edit->setFixedSize(size);
                edit->setReadOnly(true);

                std::string record_path = settings_->GetScreenRecordingPath();
                if (record_path.empty()) {
                    QString movies_path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
                    record_path = movies_path.toStdString();
                    settings_->SetScreenRecordingPath(record_path);
                }
                edit->setText(QString::fromStdString(record_path));

                layout->addWidget(edit);

                auto btn = new QPushButton("...", this);
                btn_select_screen_recording_path_ = btn;
                btn->setFixedSize(30, 30);
                btn->setToolTip(tr("Select screen recording save folder"));
                layout->addSpacing(10);
                layout->addWidget(btn);
                layout->addStretch();
                segment_layout->addSpacing(10);
                segment_layout->addLayout(layout);

                connect(btn, &QPushButton::clicked, this, [=, this]() {
                    QString dir = QFileDialog::getExistingDirectory(nullptr, "select folder", QString::fromStdString(record_path));
                    if (dir.isEmpty()) {
                        return;
                    }
                    edit->setText(dir);
                    settings_->SetScreenRecordingPath(dir.toStdString());
                });
            }
            column1_layout->addLayout(segment_layout);
        }

        column1_layout->addStretch();

        {
            auto segment_layout = new NoMarginVLayout();
            {
                // title
                auto label = new QLabel(this);
                //label->setText(tr("File Transfer"));
                label->setFixedWidth(100);
                label->setStyleSheet("font-size: 16px; font-weight: 700;");
                segment_layout->addSpacing(0);
                segment_layout->addWidget(label);
            }
            column2_layout->addLayout(segment_layout);
            column2_layout->addStretch();
        }

        setLayout(root_layout);
    }

    void StController::OnTabShow() {

    }

    void StController::OnTabHide() {

    }

}

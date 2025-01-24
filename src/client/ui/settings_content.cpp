//
// Created by RGAA on 2023/8/16.
//

#include "settings_content.h"

#include "switch_button.h"
#include "widget_helper.h"
#include "Settings.h"
#include "no_margin_layout.h"
#include "multi_display_mode_widget.h"

namespace tc
{

    SettingsContent::SettingsContent(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : AppContent(ctx, parent) {
        settings_ = Settings::Instance();

        // segment encoder
        auto tips_label_width = 220;
        auto tips_label_height = 35;
        auto tips_label_size = QSize(tips_label_width, tips_label_height);
        auto input_size = QSize(240, tips_label_height);
        auto item_margin_left = 20;

        auto root_layout = new NoMarginVLayout();
        {
            // title
            auto layout = new NoMarginHLayout();
            auto label = new QLabel(this);
            label->setText(tr("Settings"));
            label->setStyleSheet("font-size: 16px; font-weight: 700;");
            layout->addSpacing(item_margin_left);
            layout->addWidget(label);
            root_layout->addSpacing(10);
            root_layout->addLayout(layout);
        }

        {
            auto layout = new NoMarginHLayout();
            layout->addSpacing(item_margin_left);

            auto label = new QLabel(this);
            label->setFixedSize(tips_label_size);
            label->setText(tr("Audio Enabled"));
            label->setStyleSheet("font-size:14px;");
            layout->addWidget(label);

            auto switch_btn = new QCheckBox(this);
            switch_btn->setChecked(settings_->IsAudioEnabled());
            connect(switch_btn, &QCheckBox::clicked, this, [=, this](bool checked) {
                settings_->SetAudioEnabled(checked);
            });
            layout->addSpacing(30);
            layout->addWidget(switch_btn);
            layout->addStretch();

            root_layout->addLayout(layout);
        }
        {
            auto layout = new NoMarginHLayout();
            layout->addSpacing(item_margin_left);

            auto label = new QLabel(this);
            label->setFixedSize(tips_label_size);
            label->setText(tr("Clipboard Enabled"));
            label->setStyleSheet("font-size:14px;");
            layout->addWidget(label);

            auto switch_btn = new QCheckBox(this);
            switch_btn->setChecked(settings_->IsClipboardEnabled());
            connect(switch_btn, &QCheckBox::clicked, this, [=, this](bool checked) {
                settings_->SetClipboardEnabled(checked);
            });
            layout->addSpacing(30);
            layout->addWidget(switch_btn);
            layout->addStretch();

            root_layout->addLayout(layout);
        }
        if (0) {
            auto layout = new NoMarginHLayout();
            layout->addSpacing(item_margin_left);

            auto label = new QLabel(this);
            label->setFixedSize(tips_label_size);
            label->setText(tr("Multiple Monitors Display Mode"));
            label->setStyleSheet("font-size:14px;");
            layout->addWidget(label);
            layout->addStretch();
            root_layout->addLayout(layout);
        }

        if (0) {
            root_layout->addSpacing(7);
            auto layout = new QHBoxLayout();
            WidgetHelper::ClearMargin(layout);
            layout->addSpacing(item_margin_left);

            // separated
            separated_ = new MultiDisplayModeWidget(MultiDisplayMode::kSeparated, this);
            separated_->SetSelected(settings_->GetMultiDisplayMode() == MultiDisplayMode::kSeparated);
            separated_->setFixedSize(250, 150);
            separated_->SetOnClickCallback([=, this]() {
                combined_->SetSelected(false);
                settings_->SetMultiDisplayMode(MultiDisplayMode::kSeparated);
            });
            layout->addWidget(separated_);

            layout->addSpacing(20);

            combined_ = new MultiDisplayModeWidget(MultiDisplayMode::kCombined, this);
            combined_->SetSelected(settings_->GetMultiDisplayMode() == MultiDisplayMode::kCombined);
            combined_->setFixedSize(250, 150);
            combined_->SetOnClickCallback([=, this]() {
                separated_->SetSelected(false);
                settings_->SetMultiDisplayMode(MultiDisplayMode::kCombined);
            });
            layout->addWidget(combined_);

            layout->addStretch();

            root_layout->addLayout(layout);
        }

        root_layout->addStretch();
        setLayout(root_layout);
    }

    SettingsContent::~SettingsContent() {

    }

    void SettingsContent::OnContentShow() {
        AppContent::OnContentShow();
    }

    void SettingsContent::OnContentHide() {
        AppContent::OnContentHide();
    }

}
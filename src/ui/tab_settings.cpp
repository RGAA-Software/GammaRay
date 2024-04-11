//
// Created by RGAA on 2024-04-09.
//

#include "tab_settings.h"
#include "widgets/custom_tab_btn.h"
#include "widgets/no_margin_layout.h"
#include "ui/st_input.h"
#include "ui/st_media.h"
#include "ui/st_network.h"
#include "ui/st_encoder.h"

namespace tc
{

    TabSettings::TabSettings(const std::shared_ptr<Context>& ctx, QWidget* parent) : TabBase(ctx, parent) {
        auto root_layout = new NoMarginHLayout();

        auto left_button_layout = new NoMarginVLayout();
        root_layout->addSpacing(20);
        root_layout->addLayout(left_button_layout);
        auto left_area_width = 180;
        auto btn_size = QSize(left_area_width - 30, 32);
        auto btn_font_color = "#ffffff";
        int border_radius = btn_size.height()/2;
        // INPUT
        {
            auto btn = new CustomTabBtn(this);
            btn_input_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetText(tr("Input"));

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStInput);
            });
            left_button_layout->addSpacing(30);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }

        // Audio & Video
        {
            auto btn = new CustomTabBtn(this);
            btn_media_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetText(tr("Audio&Video"));

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStMedia);
            });
            left_button_layout->addSpacing(10);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }

        // Network
        {
            auto btn = new CustomTabBtn(this);
            btn_network_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetText(tr("Network"));

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStNetwork);
            });
            left_button_layout->addSpacing(10);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }

        // Encoder
        {
            auto btn = new CustomTabBtn(this);
            btn_encoder_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetText(tr("Encoder"));

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStEncoder);
            });
            left_button_layout->addSpacing(10);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }
        left_button_layout->addStretch();

        {
            // tabs
            tabs_.insert({StTabName::kStInput, new StInput(context_, this)});
            tabs_.insert({StTabName::kStMedia, new StMedia(context_, this)});
            tabs_.insert({StTabName::kStNetwork, new StNetwork(context_, this)});
            tabs_.insert({StTabName::kStEncoder, new StEncoder(context_, this)});

            tabs_[StTabName::kStInput]->SetAttach(btn_input_);
            tabs_[StTabName::kStMedia]->SetAttach(btn_media_);
            tabs_[StTabName::kStNetwork]->SetAttach(btn_network_);
            tabs_[StTabName::kStEncoder]->SetAttach(btn_encoder_);

            auto layout = new NoMarginVLayout();
            auto stack_widget = new QStackedWidget(this);
            stack_widget->addWidget(tabs_[StTabName::kStInput]);
            stack_widget->addWidget(tabs_[StTabName::kStMedia]);
            stack_widget->addWidget(tabs_[StTabName::kStNetwork]);
            stack_widget->addWidget(tabs_[StTabName::kStEncoder]);
            stacked_widget_ = stack_widget;
            layout->addWidget(stack_widget);
            root_layout->addSpacing(40);
            root_layout->addLayout(layout);
        }

        setLayout(root_layout);
        ChangeTab(StTabName::kStInput);
    }

    TabSettings::~TabSettings() {

    }

    void TabSettings::OnTabShow() {

    }

    void TabSettings::OnTabHide() {

    }

    void TabSettings::ChangeTab(const StTabName& tn) {
        for (auto& [name, tab] : tabs_) {
            if (tn == name) {
                stacked_widget_->setCurrentWidget(tabs_[tn]);
                tabs_[tn]->OnTabShow();
                ((CustomTabBtn*)tabs_[tn]->GetAttach())->ToActiveStatus();
            } else {
                tabs_[name]->OnTabShow();
                ((CustomTabBtn*)tabs_[name]->GetAttach())->ToInActiveStatus();
            }
        }
    }

}
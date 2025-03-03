//
// Created by RGAA on 2024-04-09.
//

#include "tab_settings.h"
#include "tc_qt_widget/custom_tab_btn.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "render_panel/ui/st_general.h"
#include "render_panel/ui/st_client.h"
#include "render_panel/ui/st_about_me.h"
#include "render_panel/ui/st_network.h"
#include "app_colors.h"

namespace tc
{

    TabSettings::TabSettings(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginHLayout();

        auto left_button_layout = new NoMarginVLayout();
        root_layout->addSpacing(20);
        root_layout->addLayout(left_button_layout);
        auto left_area_width = 180;
        auto btn_size = QSize(left_area_width - 30, 32);
        auto btn_font_color = "#ffffff";
        int border_radius = btn_size.height()/2;
        // General
        {
            auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
            btn_input_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetText(tr("General"));

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStGeneral);
            });
            left_button_layout->addSpacing(30);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }
        // network
        {
            auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
            btn_client_ = btn;
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
        // About me
        {
            auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
            btn_about_me_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetText(tr("AboutMe"));

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStAboutMe);
            });
            left_button_layout->addSpacing(10);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }
        left_button_layout->addStretch();

        {
            // tabs
            tabs_.insert({StTabName::kStGeneral, new StGeneral(app_, this)});
            tabs_.insert({StTabName::kStNetwork, new StNetwork(app_, this)});
            tabs_.insert({StTabName::kStAboutMe, new StAboutMe(app_, this)});

            tabs_[StTabName::kStGeneral]->SetAttach(btn_input_);
            tabs_[StTabName::kStNetwork]->SetAttach(btn_client_);
            tabs_[StTabName::kStAboutMe]->SetAttach(btn_about_me_);

            auto layout = new NoMarginVLayout();
            auto stack_widget = new QStackedWidget(this);
            stack_widget->addWidget(tabs_[StTabName::kStGeneral]);
            stack_widget->addWidget(tabs_[StTabName::kStNetwork]);
            stack_widget->addWidget(tabs_[StTabName::kStAboutMe]);
            stacked_widget_ = stack_widget;
            layout->addWidget(stack_widget);
            root_layout->addSpacing(40);
            root_layout->addLayout(layout);
        }

        setLayout(root_layout);
        ChangeTab(StTabName::kStGeneral);
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
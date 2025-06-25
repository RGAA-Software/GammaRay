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
#include "render_panel/ui/st_plugins.h"
#include "render_panel/ui/st_controller.h"
#include "app_colors.h"
#include "st_security.h"

namespace tc
{

    TabSettings::TabSettings(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginHLayout();

        auto left_button_layout = new NoMarginVLayout();
        // title margin
        left_button_layout->addSpacing(kTabContentMarginTop);

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
            btn->SetTextId("id_settings_general");

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStGeneral);
            });
            //left_button_layout->addSpacing(30);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }
        // network
        {
            auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
            btn_network_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetTextId("id_settings_network");

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStNetwork);
            });
            left_button_layout->addSpacing(10);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }
        // security
        {
            auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
            btn_security_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetTextId("id_settings_security");

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStSecurity);
            });
            left_button_layout->addSpacing(10);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }
        // plugins
        {
            auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
            btn_plugins_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetTextId("id_settings_plugins");

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStPlugins);
            });
            left_button_layout->addSpacing(10);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }
        // controller
        {
            auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
            btn_controller = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetTextId("id_settings_controller");

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStController);
            });
            left_button_layout->addSpacing(10);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }
        // other clients
        {
            auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
            btn_other_clients = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetTextId("id_settings_other_clients");

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StTabName::kStOtherClients);
            });
            left_button_layout->addSpacing(10);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }
        // About me
        {
            auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
            btn_about_me_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetTextId("id_settings_aboutme");

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
            tabs_.insert({StTabName::kStSecurity, new StSecurity(app_, this)});
            tabs_.insert({StTabName::kStPlugins, new StPlugins(app_, this)});
            tabs_.insert({StTabName::kStController, new StController(app_, this)});
            tabs_.insert({StTabName::kStOtherClients, new StClient(app_, this)});
            tabs_.insert({StTabName::kStAboutMe, new StAboutMe(app_, this)});

            tabs_[StTabName::kStGeneral]->SetAttach(btn_input_);
            tabs_[StTabName::kStNetwork]->SetAttach(btn_network_);
            tabs_[StTabName::kStSecurity]->SetAttach(btn_security_);
            tabs_[StTabName::kStPlugins]->SetAttach(btn_plugins_);
            tabs_[StTabName::kStController]->SetAttach(btn_controller);
            tabs_[StTabName::kStOtherClients]->SetAttach(btn_other_clients);
            tabs_[StTabName::kStAboutMe]->SetAttach(btn_about_me_);

            auto layout = new NoMarginVLayout();
            // title margin
            //layout->addSpacing(kTabContentMarginTop);

            auto stack_widget = new QStackedWidget(this);
            stack_widget->addWidget(tabs_[StTabName::kStGeneral]);
            stack_widget->addWidget(tabs_[StTabName::kStNetwork]);
            stack_widget->addWidget(tabs_[StTabName::kStSecurity]);
            stack_widget->addWidget(tabs_[StTabName::kStPlugins]);
            stack_widget->addWidget(tabs_[StTabName::kStController]);
            stack_widget->addWidget(tabs_[StTabName::kStOtherClients]);
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
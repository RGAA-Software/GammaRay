//
// Created by RGAA on 22/03/2025.
//

#include "tab_security_internals.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include "no_margin_layout.h"
#include "tc_pushbutton.h"
#include "tc_common_new/log.h"
#include "st_security_visitor.h"
#include "st_security_file_transfer.h"
#include "tc_qt_widget/custom_tab_btn.h"
#include "app_colors.h"
#include <Windows.h>
#include <shellapi.h>

namespace tc
{

    TabSecurityInternals::TabSecurityInternals(const std::shared_ptr<GrApplication>& app, QWidget *parent)
        : TabBase(app, parent) {
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
            btn_visitor_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetTextId("id_security_visitor");

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StSecurityTabName::kStVisitor);
            });
            //left_button_layout->addSpacing(30);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }
        // network
        {
            auto btn = new CustomTabBtn(AppColors::kTabBtnInActiveColor, AppColors::kTabBtnHoverColor, this);
            btn_file_transfer_ = btn;
            btn->SetBorderRadius(border_radius);
            btn->SetTextId("id_security_file_transfer");

            btn->SetSelectedFontColor(btn_font_color);
            btn->setFixedSize(btn_size);
            //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
            QObject::connect(btn, &QPushButton::clicked, this, [=, this]() {
                ChangeTab(StSecurityTabName::kStFileTransfer);
            });
            left_button_layout->addSpacing(10);
            left_button_layout->addWidget(btn, 0, Qt::AlignHCenter);
        }

        left_button_layout->addStretch();

        {
            // tabs
            tabs_.insert({StSecurityTabName::kStVisitor, new StSecurityVisitor(app_, this)});
            tabs_.insert({StSecurityTabName::kStFileTransfer, new StSecurityFileTransfer(app_, this)});

            tabs_[StSecurityTabName::kStVisitor]->SetAttach(btn_visitor_);
            tabs_[StSecurityTabName::kStFileTransfer]->SetAttach(btn_file_transfer_);

            auto layout = new NoMarginVLayout();
            // title margin
            //layout->addSpacing(kTabContentMarginTop);

            auto stack_widget = new QStackedWidget(this);
            stack_widget->addWidget(tabs_[StSecurityTabName::kStVisitor]);
            stack_widget->addWidget(tabs_[StSecurityTabName::kStFileTransfer]);

            stacked_widget_ = stack_widget;
            layout->addWidget(stack_widget);
            root_layout->addSpacing(40);
            root_layout->addLayout(layout);
        }

        setLayout(root_layout);
        ChangeTab(StSecurityTabName::kStVisitor);
    }

    void TabSecurityInternals::OnTabShow() {

    }

    void TabSecurityInternals::OnTabHide() {

    }

    void TabSecurityInternals::ChangeTab(const StSecurityTabName& tn) {
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
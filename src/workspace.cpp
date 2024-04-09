//
// Created by hy on 2024/4/9.
//

#include "workspace.h"
#include "application.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QStackedWidget>

#include "widgets/custom_tab_btn.h"
#include "widgets/layout_helper.h"

namespace tc
{

    Workspace::Workspace() : QMainWindow(nullptr) {
        setWindowTitle(tr("GammaRay Server"));

        // root
        auto root_layout = new QHBoxLayout();
        LayoutHelper::ClearMargins(root_layout);

        // left buttons
        {
            auto layout = new QVBoxLayout();
            LayoutHelper::ClearMargins(layout);


            // placeholder to extend the width of left area
            auto extend = new QLabel(this);
            extend->setFixedSize(170, 2);
            extend->setStyleSheet("background:#298789;");
            layout->addWidget(extend);

            // logo
            auto logo = new QLabel(this);
            logo->setFixedSize(120, 120);
            logo->setStyleSheet("background:#009988;");
            logo->setAlignment(Qt::AlignCenter);
            layout->addWidget(logo, 0, Qt::AlignHCenter);

            // buttons
            auto btn_font_color = "#ffffff";
            auto btn_size = QSize(150, 36);
            {
                auto btn = new CustomTabBtn(this);
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetText(tr("SERVER"));
                //btn->setFont(font);
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
                QObject::connect(btn, &QPushButton::clicked, this, [=]() {
                    //ChangeTab(tab_type, TabType::kInstalled);
                });
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            {
                auto btn = new CustomTabBtn(this);
                btn->SetBorderRadius(btn_size.height()/2);
                btn->SetText(tr("GAMES"));
                //btn->setFont(font);
                btn->SetSelectedFontColor(btn_font_color);
                btn->setFixedSize(btn_size);
                //tab_btns.insert(std::make_pair(TabType::kInstalled, btn));
                QObject::connect(btn, &QPushButton::clicked, this, [=]() {
                    //ChangeTab(tab_type, TabType::kInstalled);
                });
                layout->addWidget(btn, 0, Qt::AlignHCenter);
            }

            layout->addStretch();

            // version
            {

            }

            root_layout->addLayout(layout);
        }

        // right panels
        {
            auto layout = new QVBoxLayout();
            LayoutHelper::ClearMargins(root_layout);
            auto stack_widget = new QStackedWidget(this);
            layout->addWidget(stack_widget);
            root_layout->addLayout(layout);
        }

        auto root_widget = new QWidget(this);
        root_widget->setLayout(root_layout);
        setCentralWidget(root_widget);

//        app_ = std::make_shared<Application>();
//        app_->Init();
    }

}
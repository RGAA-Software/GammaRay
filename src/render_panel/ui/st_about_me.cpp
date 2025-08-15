//
// Created by RGAA on 2024-06-10.
//

#include "st_about_me.h"

#include <QPushButton>
#include <QDesktopServices>

#include "tc_label.h"
#include "version_config.h"
#include "tc_image_button.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_context.h"
#include "tc_qt_widget/no_margin_layout.h"

namespace tc
{

    StAboutMe::StAboutMe(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent){
        auto root_layout = new NoMarginVLayout();
        root_layout->addSpacing(100);
        {
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            auto label = new QLabel();
            int size = 90;
            label->setFixedSize(size, size);
            label->setScaledContents(true);
            label->setStyleSheet(R"(
                border: none;
                border-image: url(:/resources/tc_trans_icon_blue.png);
                background-repeat: no-repeat;
                background-position: center;
            )");
            layout->addWidget(label);
            layout->addStretch();
            root_layout->addLayout(layout);
        }
        {
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            auto label = new QLabel();
            label->setText("Always Online");
            label->setStyleSheet(R"(
                color: #555555;
                font-size: 14px;
                font-weight: 700;
            )");
            layout->addWidget(label);
            layout->addStretch();
            root_layout->addSpacing(10);
            root_layout->addLayout(layout);
        }
        {
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            auto label = new QLabel();
            label->setText(PROJECT_VERSION);
            label->setStyleSheet(R"(
                color: #777777;
                font-size: 13px;
            )");
            layout->addWidget(label);
            layout->addStretch();
            root_layout->addSpacing(10);
            root_layout->addLayout(layout);
        }
        {
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            auto label = new TcLabel();
            label->setStyleSheet("font-size:15px; font-weight: 700;");
            label->setFixedWidth(550);
            label->setWordWrap(true);
            label->SetTextId("id_about_me_desc");
            layout->addWidget(label);
            layout->addStretch();
            root_layout->addSpacing(50);
            root_layout->addLayout(layout);
        }

        // url
        if (0) {
            auto layout = new NoMarginHLayout();
            layout->addStretch();

            root_layout->addSpacing(50);
            root_layout->addLayout(layout);

            int size = 50;
            int normal_color = 0xffffff;
            int hover_color = 0xeeeeee;
            int press_color = 0xdddddd;
            // steam
            {
                auto label = new TcImageButton(":/resources/social/ic_steam.svg");
                label->SetColor(normal_color, hover_color, press_color);
                label->SetRoundRadius(size/2);
                label->setFixedSize(size, size);
                layout->addWidget(label);
                label->SetOnImageButtonClicked([]() {
                    QDesktopServices::openUrl(QUrl("https://store.steampowered.com/app/2184340/Rhythm_Master/"));
                });
                label->SetOnImageButtonHovering([=, this](QWidget*) {
                    this->setCursor(Qt::PointingHandCursor);
                });
                label->SetOnImageButtonLeaved([=, this](QWidget*) {
                    this->setCursor(Qt::ArrowCursor);
                });
            }
            // github
            layout->addSpacing(20);
            {
                auto label = new TcImageButton(":/resources/social/ic_github.svg");
                label->SetColor(normal_color, hover_color, press_color);
                label->SetRoundRadius(size/2);
                label->setFixedSize(size, size);
                layout->addWidget(label);
                label->SetOnImageButtonClicked([]() {
                    QDesktopServices::openUrl(QUrl("https://github.com/RGAA-Software/GammaRay"));
                });
                label->SetOnImageButtonHovering([=, this](QWidget*) {
                    this->setCursor(Qt::PointingHandCursor);
                });
                label->SetOnImageButtonLeaved([=, this](QWidget*) {
                    this->setCursor(Qt::ArrowCursor);
                });
            }
            // bilibili
            layout->addSpacing(20);
            {
                auto label = new TcImageButton(":/resources/social/ic_bilibili.svg");
                label->SetColor(normal_color, hover_color, press_color);
                label->SetRoundRadius(size/2);
                label->setFixedSize(size, size);
                layout->addWidget(label);
                label->SetOnImageButtonClicked([]() {
                    QDesktopServices::openUrl(QUrl("https://github.com/RGAA-Software/GammaRay"));
                });
                label->SetOnImageButtonHovering([=, this](QWidget*) {
                    this->setCursor(Qt::PointingHandCursor);
                });
                label->SetOnImageButtonLeaved([=, this](QWidget*) {
                    this->setCursor(Qt::ArrowCursor);
                });
            }
            // youtube
            layout->addSpacing(20);
            {
                auto label = new TcImageButton(":/resources/social/ic_youtube.svg");
                label->SetColor(normal_color, hover_color, press_color);
                label->SetRoundRadius(size/2);
                label->setFixedSize(size, size);
                layout->addWidget(label);
                label->SetOnImageButtonClicked([]() {
                    QDesktopServices::openUrl(QUrl("https://github.com/RGAA-Software/GammaRay"));
                });
                label->SetOnImageButtonHovering([=, this](QWidget*) {
                    this->setCursor(Qt::PointingHandCursor);
                });
                label->SetOnImageButtonLeaved([=, this](QWidget*) {
                    this->setCursor(Qt::ArrowCursor);
                });
            }
            // tiktok
            layout->addSpacing(20);
            {
                auto label = new TcImageButton(":/resources/social/ic_tiktok.svg");
                label->SetColor(normal_color, hover_color, press_color);
                label->SetRoundRadius(size/2);
                label->setFixedSize(size, size);
                layout->addWidget(label);
                label->SetOnImageButtonClicked([]() {
                    QDesktopServices::openUrl(QUrl("https://github.com/RGAA-Software/GammaRay"));
                });
                label->SetOnImageButtonHovering([=, this](QWidget*) {
                    this->setCursor(Qt::PointingHandCursor);
                });
                label->SetOnImageButtonLeaved([=, this](QWidget*) {
                    this->setCursor(Qt::ArrowCursor);
                });
            }

            layout->addStretch();
        }

        root_layout->addStretch();

        root_layout->addSpacing(20);
        setLayout(root_layout);
    }

    void StAboutMe::OnTabShow() {

    }

    void StAboutMe::OnTabHide() {

    }

}

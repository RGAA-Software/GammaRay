//
// Created by RGAA on 2024-06-10.
//

#include "st_about_me.h"
#include "gr_application.h"
#include "gr_context.h"
#include "tc_qt_widget/no_margin_layout.h"
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>

namespace tc
{

    StAboutMe::StAboutMe(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent){
        auto root_layout = new NoMarginVLayout();
        root_layout->addSpacing(100);
        {
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            auto label = new QLabel();
            QImage image;
            image.load(":/resources/tc_icon.png");
            auto pixmap = QPixmap::fromImage(image);
            int size = 120;
            pixmap = pixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            label->setFixedSize(size, size);
            label->setPixmap(pixmap);
            layout->addWidget(label);
            layout->addStretch();
            root_layout->addLayout(layout);
        }
        {
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            auto label = new QLabel();
            label->setStyleSheet("font-size:15px; font-weight: 700;");
            label->setFixedWidth(550);
            label->setWordWrap(true);
            label->setText(tr("GammaRay is a set of tools for streaming your games and desktop to other devices, and replaying gamepad/keyboard/mouse events in the host PC."));
            layout->addWidget(label);
            layout->addStretch();
            root_layout->addSpacing(20);
            root_layout->addLayout(layout);
        }
        // url
        int url_width = 400;
        int button_width = 150;
        {
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            auto label = new QLabel();
            label->setStyleSheet("font-size:14px; font-weight: 500;");
            label->setFixedWidth(url_width);
            label->setWordWrap(true);
            label->setText(tr("https://github.com/RGAA-Software/GammaRay"));
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
            layout->addWidget(label);

            auto btn = new QPushButton();
            btn->setFixedWidth(button_width);
            btn->setText(tr("Server Repo"));
            layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                QDesktopServices::openUrl(QUrl(label->text()));
            });

            layout->addStretch();
            root_layout->addSpacing(50);
            root_layout->addLayout(layout);
        }
        {
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            auto label = new QLabel();
            label->setStyleSheet("font-size:14px; font-weight: 500;");
            label->setFixedWidth(url_width);
            label->setWordWrap(true);
            label->setText(tr("https://github.com/RGAA-Software/GammaRayPC"));
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
            layout->addWidget(label);

            auto btn = new QPushButton();
            btn->setFixedWidth(button_width);
            btn->setText(tr("PC Client Repo"));
            layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                QDesktopServices::openUrl(QUrl(label->text()));
            });

            layout->addStretch();
            root_layout->addSpacing(5);
            root_layout->addLayout(layout);
        }
        {
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            auto label = new QLabel();
            label->setStyleSheet("font-size:14px; font-weight: 500;");
            label->setFixedWidth(url_width);
            label->setWordWrap(true);
            label->setText(tr("https://github.com/RGAA-Software/GammaRayAndroid"));
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
            layout->addWidget(label);

            auto btn = new QPushButton();
            btn->setText(tr("Android Client Repo"));
            btn->setFixedWidth(button_width);
            layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                QDesktopServices::openUrl(QUrl(label->text()));
            });

            layout->addStretch();
            root_layout->addSpacing(5);
            root_layout->addLayout(layout);
        }
        root_layout->addStretch();

        {
            auto layout = new NoMarginHLayout();
            root_layout->addLayout(layout);
            layout->addStretch();

            license_ = new QLabel(this);
            license_->setText("License[Apache]");
            license_->setStyleSheet(R"(font-size:25px; color:#bb2222;)");
            layout->addWidget(license_);
            layout->addSpacing(20);
        }

        root_layout->addSpacing(20);
        setLayout(root_layout);
    }

    void StAboutMe::OnTabShow() {

    }

    void StAboutMe::OnTabHide() {

    }

}

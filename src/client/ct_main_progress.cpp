//
// Created by RGAA on 20/02/2025.
//

#include "ct_main_progress.h"
#include <QPainter>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include "tc_qt_widget/no_margin_layout.h"

namespace tc
{

    MainProgress::MainProgress(const std::shared_ptr<ThunderSdk>& sdk, QWidget* parent) : QLabel(parent) {
        sdk_ = sdk;
        auto root_layout = new NoMarginVLayout();
        root_layout->addStretch(1);
        // logo
        {
            auto layout = new NoMarginHLayout();
            auto logo = new QLabel(this);
            logo->setFixedSize(160, 160);
            logo->setStyleSheet(R"(border-image: url(:/resources/image/tc_icon_raw.png)})");
            layout->addStretch();
            layout->addWidget(logo);
            layout->addStretch();

            root_layout->addLayout(layout);
        }
        // title
        {
            auto layout = new NoMarginHLayout();
            auto title = new QLabel(this);
            lbl_main_message_ = title;
            title->setText(tr("Connecting Server..."));
            title->setStyleSheet(R"(font-size: 20px; font-weight: 500;)");
            layout->addStretch();
            layout->addWidget(title);
            layout->addStretch();
            root_layout->addSpacing(40);
            root_layout->addLayout(layout);
        }

        // progress bar
        {
            auto layout = new NoMarginHLayout();
            auto progress_bar = new QProgressBar();
            progress_bar_ = progress_bar;
            progress_bar->setFixedSize(400, 6);
            progress_bar->setMaximum(4);
            progress_bar->setValue(3);
            layout->addStretch();
            layout->addWidget(progress_bar);
            layout->addStretch();

            root_layout->addSpacing(20);
            root_layout->addLayout(layout);
        }

        // sub messages
        {
            auto layout = new NoMarginHLayout();
            auto lbl = new QLabel(this);
            lbl_sub_message_ = lbl;
            lbl->setText(tr("checking connection..."));
            lbl->setStyleSheet(R"(font-size: 16px;;)");
            layout->addStretch();
            layout->addWidget(lbl);
            layout->addStretch();
            root_layout->addSpacing(20);
            root_layout->addLayout(layout);
        }

        // cancel button
        {
            auto layout = new NoMarginHLayout();
            auto lbl = new QPushButton(this);
            lbl->setFixedSize(150, 30);
            lbl->setText(tr("CANCEL"));
            layout->addStretch();
            layout->addWidget(lbl);
            layout->addStretch();
            root_layout->addSpacing(20);
            root_layout->addLayout(layout);

            connect(lbl, &QPushButton::clicked, this, [=, this]() {

            });
        }

        root_layout->addStretch(5);
        setLayout(root_layout);

        QImage image;
        image.load(":/resources/image/ic_loading_bg.svg");
        bg_pixmap_ = QPixmap::fromImage(image);
        bg_pixmap_ = bg_pixmap_.scaled(640, 640);
    }

    void MainProgress::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(0xffffff));
        painter.drawRect(this->rect());

        if (!bg_pixmap_.isNull()) {
            painter.drawPixmap((this->width() - bg_pixmap_.width())/2, (this->height() - bg_pixmap_.height())/2, bg_pixmap_);
        }
        QLabel::paintEvent(event);
    }

}
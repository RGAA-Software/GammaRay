//
// Created by RGAA on 2024-04-11.
//

#include "st_client.h"

#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_common_new/qrcode/qr_generator.h"
#include "tc_common_new/folder_util.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"

namespace tc
{

    StClient::StClient(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginVLayout();
        root_layout->addStretch();
        auto content_layout = new NoMarginHLayout();
        content_layout->addStretch();
        auto exeDir = QApplication::applicationDirPath().toStdString();
        {
            auto item_layout = new NoMarginVLayout();

            auto logo = new QLabel();
            logo->setFixedSize(128, 128);
            QImage image;
            image.load(":/icons/logo_windows.png");
            logo->setPixmap(QPixmap::fromImage(image));
            item_layout->addWidget(logo);

            auto btn = new QPushButton(this);
            btn->setFixedSize(120, 35);
            btn->setText(tr("Open Directory"));
            item_layout->addSpacing(15);
            item_layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=]() {
                FolderUtil::OpenDir(std::format("{}/clients/windows", exeDir));
            });

            auto qrcode = new QLabel(this);
            qrcode->setFixedSize(128, 128);
            item_layout->addSpacing(15);
            item_layout->addWidget(qrcode);

            content_layout->addLayout(item_layout);
        }
        {
            auto item_layout = new NoMarginVLayout();

            auto logo = new QLabel();
            logo->setFixedSize(128, 128);
            QImage image;
            image.load(":/icons/logo_android.png");
            logo->setPixmap(QPixmap::fromImage(image));
            item_layout->addWidget(logo);

            auto btn = new QPushButton(this);
            btn->setFixedSize(120, 35);
            btn->setText(tr("Open Directory"));
            item_layout->addSpacing(15);
            item_layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=]() {
                FolderUtil::OpenDir(std::format("{}/clients/android", exeDir));
            });

            auto qrcode = new QLabel(this);
            qrcode->setFixedSize(128, 128);
            item_layout->addSpacing(15);
            item_layout->addWidget(qrcode);
            auto ips = app_->GetContext()->GetIps();
            if (!ips.empty()) {
                auto url = std::format("http://{}:{}/clients/android/a.webp", (*ips.begin()).ip_addr_, GrSettings::Instance()->http_server_port_);
                auto qr_pixmap = QrGenerator::GenQRPixmap(url.c_str(), 128);
                qrcode->setPixmap(qr_pixmap);
            }
            content_layout->addSpacing(40);
            content_layout->addLayout(item_layout);
        }
        content_layout->addStretch();
        root_layout->addLayout(content_layout);
        root_layout->addStretch();
        setLayout(root_layout);
    }

    void StClient::OnTabShow() {

    }

    void StClient::OnTabHide() {

    }

}
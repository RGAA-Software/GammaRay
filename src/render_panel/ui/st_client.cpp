//
// Created by RGAA on 2024-04-11.
//

#include "st_client.h"

#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include "tc_pushbutton.h"
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
        content_layout->addStretch(1000);
        auto exeDir = QApplication::applicationDirPath().toStdString();
        auto icon_size = QSize(60, 60);
        {
            auto item_layout = new NoMarginVLayout();

            auto logo_layout = new NoMarginHLayout();
            auto logo = new QLabel();
            logo->setFixedSize(icon_size);
            logo->setStyleSheet(R"(
                border: none;
                border-image: url(:/resources/image/ic_android.svg);
                background-repeat: no-repeat;
                background-position: center;
            )");
            logo_layout->addStretch();
            logo_layout->addWidget(logo, Qt::AlignHCenter);
            logo_layout->addStretch();
            item_layout->addLayout(logo_layout);

            auto btn = new TcPushButton(this);
            btn->setFixedSize(120, 35);
            btn->SetTextId("id_open_directory");
            item_layout->addSpacing(25);
            item_layout->addWidget(btn);
            connect(btn, &QPushButton::clicked, this, [=]() {
                FolderUtil::OpenDir(std::format("{}/clients/android", exeDir));
            });

            auto qrcode = new QLabel(this);
            qrcode->setFixedSize(128, 128);
            item_layout->addSpacing(15);
            item_layout->addWidget(qrcode);
            //auto ips = app_->GetContext()->GetIps();
            //if (!ips.empty()) {
            //    auto url = std::format("http://{}:{}/clients/android/a.webp", (*ips.begin()).ip_addr_, GrSettings::Instance()->GetPanelServerPort());
            //    auto qr_pixmap = QrGenerator::GenQRPixmap(url.c_str(), 128);
            //    qrcode->setPixmap(qr_pixmap);
            //}
            content_layout->addLayout(item_layout);
        }
        content_layout->addSpacing(40);
        {
            auto item_layout = new NoMarginVLayout();
            auto logo_layout = new NoMarginHLayout();
            auto logo = new QLabel();
            logo->setFixedSize(icon_size);
            logo->setStyleSheet(R"(
                border: none;
                border-image: url(:/resources/image/ic_ios.svg);
                background-repeat: no-repeat;
                background-position: center;
            )");
            logo_layout->addStretch();
            logo_layout->addWidget(logo, Qt::AlignHCenter);
            logo_layout->addStretch();
            item_layout->addLayout(logo_layout);

            auto btn = new TcPushButton(this);
            btn->setEnabled(false);
            btn->setFixedSize(120, 35);
            btn->SetTextId("id_open_directory");
            item_layout->addSpacing(25);
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
        content_layout->addStretch(1000);
        root_layout->addLayout(content_layout);
        root_layout->addStretch();
        setLayout(root_layout);
    }

    void StClient::OnTabShow() {

    }

    void StClient::OnTabHide() {

    }

}
//
// Created by hy on 2024/4/9.
//

#include "tab_server.h"

#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <boost/format.hpp>
#include <utility>

#include <boost/format.hpp>
#include "context.h"
#include "widgets/main_item_delegate.h"
#include "qrcode/qr_generator.h"
#include "widgets/layout_helper.h"
#include "widgets/no_margin_layout.h"
#include "widgets/round_img_display.h"

namespace tc
{

    TabServer::TabServer(const std::shared_ptr<Context>& ctx, QWidget *parent) : TabBase(ctx, parent) {
        auto broadcast_msg = ctx->MakeBroadcastMessage();
        qr_pixmap_ = QrGenerator::GenQRPixmap(broadcast_msg.c_str(), 200);
        // root layout
        auto root_layout = new QVBoxLayout();
        LayoutHelper::ClearMargins(root_layout);

        // content layout
        auto content_layout = new QHBoxLayout();
        LayoutHelper::ClearMargins(content_layout);

        // left part
        {
            auto left_layout = new NoMarginHLayout();
            content_layout->addLayout(left_layout);
            content_layout->addSpacing(20);

            // machine code
            {
                auto layout = new NoMarginVLayout();
                layout->addSpacing(30);

                int size = 140;
                auto img_path = std::format(":/icons/{}.png", std::atoi(context_->GetSysUniqueId().c_str())%30+1);
                auto avatar = new RoundImageDisplay(img_path.c_str(), size, size, size/2);
                layout->addWidget(avatar, 0, Qt::AlignHCenter);

                auto title = new QLabel(this);
                title->setFixedWidth(250);
                title->setText(tr("Machine Code"));
                title->setAlignment(Qt::AlignCenter);
                title->setStyleSheet(R"(font-size: 15px;)");
                layout->addSpacing(15);
                layout->addWidget(title, 0, Qt::AlignHCenter);

                auto msg = new QLabel(this);
                msg->setText(context_->GetSysUniqueId().c_str());
                msg->setStyleSheet(R"(font-size: 40px; font-family: ScreenMatrix;)");
                layout->addSpacing(8);
                layout->addWidget(msg, 0, Qt::AlignHCenter);
                layout->addStretch();
                left_layout->addLayout(layout);
            }

            left_layout->addSpacing(15);

            {
                auto layout = new NoMarginVLayout();
                layout->addSpacing(20);
                auto title = new QLabel(this);
                title->setFixedWidth(qr_pixmap_.width());
                title->setAlignment(Qt::AlignCenter);
                title->setText(tr("Server QR Code"));
                title->setStyleSheet(R"(font-size: 15px;)");
                layout->addWidget(title);

                auto qr_info = new QLabel(this);
                qr_info->setPixmap(qr_pixmap_);
                layout->addSpacing(9);
                layout->addWidget(qr_info);
                layout->addStretch();
                left_layout->addLayout(layout);
            }
        }

        // right part
        {
            auto layout = new QVBoxLayout();
            LayoutHelper::ClearMargins(layout);
            content_layout->addLayout(layout);
        }

        //
        content_layout->addStretch();

        root_layout->addLayout(content_layout);
        setLayout(root_layout);
    }

    TabServer::~TabServer() {

    }

    void TabServer::OnTabShow() {
        TabBase::OnTabShow();
    }

    void TabServer::OnTabHide() {
        TabBase::OnTabHide();
    }


}
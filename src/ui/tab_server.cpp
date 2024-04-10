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
#include <QScrollbar>
#include <QMenu>
#include <QAction>
#include <boost/format.hpp>
#include <utility>

#include <boost/format.hpp>
#include "context.h"
#include "widgets/main_item_delegate.h"
#include "qrcode/qr_generator.h"
#include "widgets/layout_helper.h"

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
            auto layout = new QVBoxLayout();
            LayoutHelper::ClearMargins(layout);
            content_layout->addLayout(layout);

            auto qr_info = new QLabel(this);
            layout->addWidget(qr_info);
            qr_info->setPixmap(qr_pixmap_);
        }

        // right part
        {
            auto layout = new QVBoxLayout();
            LayoutHelper::ClearMargins(layout);
            content_layout->addLayout(layout);
        }

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
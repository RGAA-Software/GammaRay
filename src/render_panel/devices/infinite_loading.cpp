//
// Created by RGAA on 21/05/2025.
//

#include "infinite_loading.h"
#include <QResizeEvent>
#include <QPainter>
#include "tc_label.h"
#include "widget_helper.h"
#include "no_margin_layout.h"
#include "tc_common_new/uid_spacer.h"
#include "tc_spvr_client/spvr_stream.h"
#include "render_panel/gr_application.h"
#include "skin/interface/skin_interface.h"
#include "client/ct_stream_item_net_type.h"
#include "tc_qt_widget/loadings/winstyle/win10circleloadingwidget.h"
#include "tc_qt_widget/loadings/winstyle/win10horizontalloadingwidget.h"

namespace tc
{

    InfiniteLoading::InfiniteLoading(const std::shared_ptr<GrContext>& ctx, const QString& msg)
        : QDialog(nullptr) {

        setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog);
        setAttribute(Qt::WA_TranslucentBackground);
        //this->setStyleSheet("background:#00000000;");
        setFixedSize(320, 240);
        auto root_layout = new NoMarginVLayout;

        {
            int size = 70;
            auto lbl = new QLabel(this);
            lbl->setStyleSheet("font-weight: bold; font-size: 15px; color: #555555;");
            lbl->setText(msg);
            root_layout->addSpacing(70);

            auto layout = new NoMarginHLayout();
            layout->addStretch();
            layout->addWidget(lbl);
            layout->addStretch();

            root_layout->addLayout(layout);
        }

        {
            auto layout = new NoMarginHLayout;
            h_loading_widget_ = new Win10CircleLoadingWidget(this);
            h_loading_widget_->setFixedSize(50, 50);
            h_loading_widget_->show();
            h_loading_widget_->setBackgroundColor(QColor("#ffffff"));
            h_loading_widget_->setItemLength(8);
            h_loading_widget_->setExtendDuration(240);
            h_loading_widget_->setItemCount(6);
            h_loading_widget_->setDuration(1000);
            QList<QColor> colors;
            colors << QColor("#2962FF") << QColor("#2979FF")
                   << QColor("#448AFF") << QColor("#82B1FF")
                   << QColor("#90CAF9") << QColor("#A0DAF9");
            h_loading_widget_->setItemColors(colors);
            h_loading_widget_->updateFrameData();
            h_loading_widget_->start();

            layout->addStretch();
            layout->addWidget(h_loading_widget_);
            layout->addStretch();

            root_layout->addSpacing(30);
            root_layout->addLayout(layout);
            root_layout->addStretch();
        }

        setLayout(root_layout);

        //
        WidgetHelper::AddShadow(this, 0x666666, 30);
    }

    void InfiniteLoading::resizeEvent(QResizeEvent *event) {
        QWidget::resizeEvent(event);
    }

    void InfiniteLoading::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(0xffffff));

        int offset = 30;
        int radius = 7;
        QRect rect(offset, offset, this->width() - offset*2, this->height()-offset*2);
        painter.drawRoundedRect(rect, radius, radius);
    }

    void InfiniteLoading::Close() {
        h_loading_widget_->stop();
        this->close();
    }

}
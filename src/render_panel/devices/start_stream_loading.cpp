//
// Created by RGAA on 21/05/2025.
//

#include "start_stream_loading.h"
#include <QResizeEvent>
#include <QLabel>
#include <QPainter>
#include "render_panel/database/stream_item.h"
#include "no_margin_layout.h"
#include "widget_helper.h"
#include "tc_qt_widget/loadings/winstyle/win10circleloadingwidget.h"
#include "tc_qt_widget/loadings/winstyle/win10horizontalloadingwidget.h"

namespace tc
{

    StartStreamLoading::StartStreamLoading(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<StreamItem>& item)
        : QDialog(nullptr) {

        setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog);
        setAttribute(Qt::WA_TranslucentBackground);
        //this->setStyleSheet("background:#00000000;");
        setFixedSize(480, 320);
        auto root_layout = new NoMarginVLayout;

        {
            //
            QImage image(":/resources/tc_icon.png");
            auto pixmap = QPixmap::fromImage(image);
            int size = 80;
            pixmap = pixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            auto lbl_icon = new QLabel(this);
            lbl_icon->setFixedSize(size, size);
            lbl_icon->setPixmap(pixmap);
            root_layout->addSpacing(50);

            auto layout = new NoMarginHLayout();
            layout->addStretch();
            layout->addWidget(lbl_icon);
            layout->addStretch();

            root_layout->addLayout(layout);
        }

        {
            auto title = [=]() -> std::string {
                if (!item->stream_name_.empty()) {
                    return item->stream_name_;
                }
                else if (!item->remote_device_id_.empty()) {
                    return item->remote_device_id_;
                }
                else if (!item->stream_host_.empty()) {
                    return item->stream_host_;
                }
                else {
                    return "Remote Device";
                }
            }();

            auto lbl_title = new QLabel(std::format("Start Streaming <span style=\"color:#2979ff;\">{}</span>", title).c_str());
            lbl_title->setFixedWidth(this->width());
            lbl_title->setAlignment(Qt::AlignCenter);
            lbl_title->setStyleSheet("font-size: 15px; font-weight:bold; color: #555555;");
            root_layout->addSpacing(20);
            root_layout->addWidget(lbl_title);
        }

        {
            auto layout = new NoMarginHLayout;
            h_loading_widget_ = new Win10HorizontalLoadingWidget(this);
            h_loading_widget_->setFixedSize(250, 60);
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

            root_layout->addSpacing(15);
            root_layout->addLayout(layout);
            root_layout->addStretch();
        }

        setLayout(root_layout);

        //
        WidgetHelper::AddShadow(this, 0x666666, 30);
    }

    void StartStreamLoading::resizeEvent(QResizeEvent *event) {
        QWidget::resizeEvent(event);
    }

    void StartStreamLoading::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(0xffffff));

        int offset = 30;
        int radius = 7;
        QRect rect(offset, offset, this->width() - offset*2, this->height()-offset*2);
        painter.drawRoundedRect(rect, radius, radius);
    }

}
//
// Created by RGAA on 2023/8/19.
//

#include "stream_item_widget.h"

#include <QPainter>
#include <QPainterPath>

#include "app_color_theme.h"

namespace tc
{

    StreamItemWidget::StreamItemWidget(const StreamItem& item, int bg_color, QWidget* parent) : QWidget(parent) {
        this->item_ = item;
        this->bg_color_ = bg_color;
        icon_ = QPixmap::fromImage(QImage(":/resources/image/windows.svg"));
        icon_ = icon_.scaled(icon_.width()/3, icon_.height()/3, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    StreamItemWidget::~StreamItemWidget() {

    }

    void StreamItemWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing, true);
        painter.setRenderHints(QPainter::TextAntialiasing, true);
        painter.setRenderHints(QPainter::SmoothPixmapTransform, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(0xffffff));
        painter.drawRect(0, 0, QWidget::width(), QWidget::height());
        painter.setBrush(QBrush(QColor(bg_color_)));
        painter.drawRoundedRect(0, 0, width(), height(), radius_, radius_);

        int border_width = 2;
        {
            int bottom_region_height = 32;
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(QColor(0x33, 0x33, 0x33)));
            painter.drawRoundedRect(0, this->height() - bottom_region_height, this->width()-border_width, bottom_region_height, radius_, radius_);
            painter.drawRect(0, this->height() - bottom_region_height, this->width()-border_width, bottom_region_height/2);

            auto font = painter.font();
            font.setBold(true);
            font.setPointSize(11);
            painter.setFont(font);
            painter.setPen(QPen(QColor(0xffffff)));
            painter.drawText(QRect(10, this->height()-bottom_region_height, this->width(), bottom_region_height), Qt::AlignVCenter, item_.stream_name.c_str());
        }

        QPen pen;
        if (enter_) {
            pen.setColor(QColor(AppColorTheme::kAppMenuItemBgHoverColor));
        } else {
            pen.setColor(QColor(0xffffff));
        }
        pen.setWidth(border_width);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(border_width/2, border_width/2, width() - border_width, height() - border_width, radius_-2, radius_-2);

        int margin = 20;
        painter.drawPixmap(QWidget::width() - icon_.width() - margin, margin, icon_);
    }

    void StreamItemWidget::enterEvent(QEnterEvent *event) {
        enter_ = true;
        update();
    }

    void StreamItemWidget::leaveEvent(QEvent *event) {
        enter_ = false;
        update();
    }

}
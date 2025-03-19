//
// Created by RGAA on 2023/8/19.
//

#include "stream_item_widget.h"

#include <QPainter>
#include <QPainterPath>
#include "app_color_theme.h"
#include "tc_common_new/uid_spacer.h"

namespace tc
{

    StreamItemWidget::StreamItemWidget(const StreamItem& item, int bg_color, QWidget* parent) : QWidget(parent) {
        this->item_ = item;
        this->bg_color_ = bg_color;
        this->setStyleSheet("background:#00000000;");
        if (icon_.isNull()) {
            icon_ = QPixmap::fromImage(QImage(":/resources/image/windows.svg"));
            icon_ = icon_.scaled(icon_.width() / 3.8, icon_.height() / 3.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        if (bg_pixmap_.isNull()) {
            bg_pixmap_ = QPixmap::fromImage(QImage(":/resources/image/test_cover.jpg"));
        }
    }

    StreamItemWidget::~StreamItemWidget() {

    }

    void StreamItemWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing, true);
        painter.setRenderHints(QPainter::TextAntialiasing, true);
        painter.setRenderHints(QPainter::SmoothPixmapTransform, true);
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(0xffffff));
            painter.drawRoundedRect(0, 0, width(), height(), radius_, radius_);
        }

        {
            painter.save();
            int width = this->width();
            QRect info_rect(0, 0, width, 155);

            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(info_rect, radius_, radius_);
            QRect temp_rect(info_rect.left(), info_rect.top()+info_rect.height()/2, info_rect.width(), info_rect.height()/2);
            path.addRect(temp_rect);
            painter.fillPath(path,  QBrush(QColor(93, 201, 87)));
            path.closeSubpath();

            painter.setClipPath(path);
            painter.drawPixmap(info_rect, bg_pixmap_);

            painter.setBrush(QBrush(QColor(0xff, 0xff, 0xff, 0xee)));
            painter.drawPath(path);
            painter.restore();
        }

        int border_width = 2;
        {
            auto font = painter.font();
            font.setBold(true);
            font.setPointSize(15);
            painter.setFont(font);
            painter.setPen(QPen(QColor(0x333333)));
            std::string stream_name = item_.stream_name;
            painter.drawText(QRect(20, 0, this->width(), 50), Qt::AlignVCenter, stream_name.c_str());
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
        painter.drawPixmap(QWidget::width() - icon_.width() - margin, 85, icon_);
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
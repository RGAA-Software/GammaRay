//
// Created by RGAA on 2023-09-02.
//

#include "multi_display_mode_widget.h"
#include "app_color_theme.h"

namespace tc
{

    MultiDisplayModeWidget::MultiDisplayModeWidget(MultiDisplayMode mode, QWidget* parent) : QWidget(parent) {
        display_mode_ = mode;
    }

    MultiDisplayModeWidget::~MultiDisplayModeWidget() {

    }

    void MultiDisplayModeWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int bg_gap = 1;
        int radius = 10;
        if (enter_ || selected_) {
            QPen pen(AppColorTheme::kAppMenuItemBgPressColor);
            pen.setWidth(2);
            painter.setPen(pen);
            painter.setBrush(QColor(0xC0DCF2));
        }
        else {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0xffffff));
        }
        painter.drawRoundedRect(bg_gap, bg_gap, this->width()-2*bg_gap, this->height()-2*bg_gap, radius, radius);

        QPen pen((QColor(AppColorTheme::kAppMenuItemBgPressColor)));
        pen.setWidth(1);
        painter.setPen(pen);

        int solid_color = AppColorTheme::kAppMenuItemBgPressColor;

        auto brush = QBrush(solid_color);
        brush.setStyle(Qt::BrushStyle::BDiagPattern);
        painter.setBrush(brush);

        //painter.setBrush(Qt::NoBrush);

        if (display_mode_ == MultiDisplayMode::kSeparated) {
            int x_border_gap = 13;
            int y_border_gap = 10;
            int width = (this->width() - x_border_gap*3)/2;
            painter.drawRoundedRect(x_border_gap, y_border_gap, width, this->height()-y_border_gap*2, radius, radius);
            painter.drawRoundedRect(x_border_gap*2 + width, y_border_gap, width, this->height()-y_border_gap*2, radius, radius);

            painter.setBrush(QBrush(solid_color));
            painter.drawEllipse(QPoint(this->width() - 2*x_border_gap - width - 16, y_border_gap + 16), 8, 8);
            painter.drawEllipse(QPoint(this->width() - x_border_gap - 16, y_border_gap + 16), 8, 8);
        }
        else if (display_mode_ == MultiDisplayMode::kCombined) {
            int x_border_gap = 13;
            int y_border_gap = 10;
            painter.drawRoundedRect(x_border_gap, y_border_gap, this->width()-x_border_gap*2, this->height() - y_border_gap*2, radius, radius);
            painter.setBrush(QBrush(solid_color));
            painter.drawEllipse(QPoint(this->width() - x_border_gap - 16, y_border_gap + 16), 8, 8);
        }
    }

    void MultiDisplayModeWidget::enterEvent(QEnterEvent *event) {
        enter_ = true;
        update();
    }

    void MultiDisplayModeWidget::leaveEvent(QEvent *event) {
        enter_ = false;
        update();
    }

    void MultiDisplayModeWidget::mouseReleaseEvent(QMouseEvent *event) {
        if (click_cbk_) {
            click_cbk_();
        }
        SetSelected(true);
    }

    void MultiDisplayModeWidget::SetSelected(bool selected) {
        selected_ = selected;
        update();
    }

}
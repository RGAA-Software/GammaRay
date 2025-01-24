//
// Created by RGAA on 17/08/2024.
//

#include <QPainterPath>
#include "computer_icon.h"

namespace tc
{

    ComputerIcon::ComputerIcon(const std::shared_ptr<ClientContext>& ctx, int idx, QWidget* parent) : BaseWidget(ctx, parent) {
        QImage image;
        image.load(":resources/image/ic_computer.svg");
        pixmap_ = QPixmap::fromImage(image);
        pixmap_ = pixmap_.scaled(icon_size_, icon_size_, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        monitor_index_ = idx;
    }

    void ComputerIcon::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);

        QColor bg_color;
        QColor border_color;
        if (pressed_) {
            bg_color = QColor(0xdddddd);
            border_color = QColor(0xbbbbbb);
        } else if (enter_) {
            bg_color = QColor(0xeeeeee);
            border_color = QColor(0xbbbbbb);
        } else {
            bg_color = QColor(0xffffff);
            border_color = QColor(0xbbbbbb);
        }

        painter.setBrush(QBrush(bg_color));
        QPen pen(border_color);
        pen.setWidth(1);
        painter.setPen(pen);
        float radius = 5;

        QPainterPath path;
        path.addRoundedRect(0, 0, width() - 1, height() - 1, radius, radius);
        path.translate(0.5, 0.5);
        painter.drawPath(path);

        painter.drawPixmap((this->width()-icon_size_)/2, (this->height()-icon_size_)/2, pixmap_);
        QFont font = painter.font();
        font.setPointSize(7);
        painter.setFont(font);
        painter.setPen(QPen(0x333333));
        painter.drawText(QRect(rect().x(), rect().y() - 1, rect().width(), rect().height()), Qt::AlignCenter, std::to_string(monitor_index_+1).c_str());

        if (selected_) {
            painter.setPen(QPen(0x333333));
            painter.setBrush(QBrush(0x00ff00));
            int indicator_size = 8;
            int offset = 1;
            painter.drawRoundedRect(width()-indicator_size-offset, height()-indicator_size-offset, indicator_size, indicator_size, indicator_size/2, indicator_size/2);
        }

    }

    void ComputerIcon::enterEvent(QEnterEvent *event) {
        enter_ = true;
        repaint();
    }

    void ComputerIcon::leaveEvent(QEvent *event) {
        enter_ = false;
        repaint();
    }

    void ComputerIcon::mousePressEvent(QMouseEvent *event) {
        pressed_ = true;
        repaint();
    }

    void ComputerIcon::mouseReleaseEvent(QMouseEvent *event) {
        pressed_ = false;
        repaint();
        if (click_listener_) {
            click_listener_(this);
        }
    }

    void ComputerIcon::UpdateSelectedState(bool selected) {
        selected_ = selected;
        repaint();
    }

    void ComputerIcon::SetMonitorName(const std::string& name) {
        monitor_name_ = name;
    }

    int ComputerIcon::GetMonitorIndex() const {
        return monitor_index_;
    }

    std::string ComputerIcon::GetMonitorName() {
        return monitor_name_;
    }

    bool ComputerIcon::IsSelected() {
        return selected_;
    }
}
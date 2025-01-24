//
// Created by RGAA on 6/07/2024.
//

#include "float_icon.h"

namespace tc
{

    FloatIcon::FloatIcon(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {

    }

    void FloatIcon::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        int offset = 1;
        painter.setPen(QPen(0xcccccc));
        if (pressed_) {
            painter.setBrush(QBrush(0xd9d9d9));
        } else if (enter_) {
            painter.setBrush(QBrush(0xf0f0f0));
        } else {
            painter.setBrush(QBrush(0xf8f8f8));
        }
        painter.drawRoundedRect(offset, offset, this->width()-2*offset, this->height()-2*offset, this->width()/2, this->height()/2);
        auto target_pixmap = is_selected_ ? selected_pixmap_ : normal_pixmap_;
        if (!target_pixmap.isNull()) {
            painter.drawPixmap((this->width() - target_pixmap.width()) / 2,
                               (this->height() - target_pixmap.height()) / 2,
                               target_pixmap.width(), target_pixmap.height(), target_pixmap);
        }
    }

    void FloatIcon::enterEvent(QEnterEvent *event) {
        enter_ = true;
        repaint();
    }

    void FloatIcon::leaveEvent(QEvent *event) {
        enter_ = false;
        repaint();
    }

    void FloatIcon::mousePressEvent(QMouseEvent *event) {
        pressed_ = true;
        repaint();
    }

    void FloatIcon::mouseReleaseEvent(QMouseEvent *event) {
        pressed_ = false;
        repaint();
        if (click_listener_) {
            click_listener_(this);
        }
    }

    void FloatIcon::SwitchToNormalState() {
        is_selected_ = false;
        repaint();
    }

    void FloatIcon::SwitchToSelectedState() {
        is_selected_ = true;
        repaint();
    }

    void FloatIcon::SetIcons(const QString& normal_path, const QString& selected_path) {
        auto icon_size = QSize(this->width()-10, this->height()-10);
        {
            QImage image;
            image.load(normal_path);
            normal_pixmap_ = QPixmap::fromImage(image);
            normal_pixmap_ = normal_pixmap_.scaled(icon_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        {
            QImage image;
            image.load(selected_path);
            selected_pixmap_ = QPixmap::fromImage(image);
            selected_pixmap_ = selected_pixmap_.scaled(icon_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        repaint();
    }

}

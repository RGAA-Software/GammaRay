//
// Created by RGAA on 6/07/2024.
//

#include "float_notification_handle.h"
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>

namespace tc
{

    FloatNotificationHandle::FloatNotificationHandle(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        setFixedSize(40, 50);
        this->setStyleSheet(R"( background-color: #00000000; )");
        auto ps = new QGraphicsDropShadowEffect();
        ps->setBlurRadius(20);
        ps->setOffset(0, 0);
        ps->setColor(0xaaaaaa);
        this->setGraphicsEffect(ps);
    }

    void FloatNotificationHandle::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        if (pressed_) {
            painter.setBrush(QBrush(0xd9d9d9));
        } else if (enter_) {
            painter.setBrush(QBrush(0xf0f0f0));
        } else {
            painter.setBrush(QBrush(0xf8f8f8));
        }
        QPainterPath path;
        path.moveTo(this->width()/2, this->height()/2);
        int offset = 2;
        path.arcTo(QRect(offset, 0, this->width()-offset*2, this->height()), 90, 180);
        painter.drawPath(path);

        if (!pixmap_.isNull()) {
            painter.drawPixmap((this->width()/2 - pixmap_.width())/2 + offset, (this->height()-pixmap_.height())/2, pixmap_);
        }
    }

    void FloatNotificationHandle::SetPixmap(const QString& path) {
        QImage image;
        image.load(path);
        pixmap_ = QPixmap::fromImage(image);
        if (!pixmap_.isNull()) {
            pixmap_ = pixmap_.scaled(15, 15, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            repaint();
        }
    }

    void FloatNotificationHandle::enterEvent(QEnterEvent *event) {
        enter_ = true;
        repaint();
    }

    void FloatNotificationHandle::leaveEvent(QEvent *event) {
        enter_ = false;
        repaint();
    }

    void FloatNotificationHandle::mousePressEvent(QMouseEvent *event) {
        pressed_ = true;
        repaint();
    }

    void FloatNotificationHandle::mouseReleaseEvent(QMouseEvent *event) {
        pressed_ = false;
        repaint();
        if (click_listener_) {
            click_listener_(this);
        }
    }

    void FloatNotificationHandle::SetOnClickListener(OnClickListener&& l) {
        click_listener_ = l;
    }

    void FloatNotificationHandle::SetExpand(bool exp) {
        expand_ = exp;
    }

}

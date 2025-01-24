//
// Created by RGAA on 2023-09-01.
//

#include "switch_button.h"
#include <QVariant>
#include <QTimer>
#include <QPropertyAnimation>

namespace tc
{

    SwitchButton::SwitchButton(QWidget *parent) : QWidget(parent) {

    }

    SwitchButton::~SwitchButton() {

    }

    void SwitchButton::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // bg
        QPen pen((QColor(border_color)));
        painter.setPen(pen);
        painter.setBrush(QColor(normal_bg_color_));

        int border_radius = this->height() / 2;
        painter.drawRoundedRect(border_width, border_width, this->width() - 2 * border_width,
                                this->height() - 2 * border_width, border_radius, border_radius);

        // thumb
        painter.setPen(Qt::NoPen);
        int thumb_size = this->height() - border_width * 2 - 6;
        int circle_radius = thumb_size / 2;

        right_point_ = this->width() - (this->height() / 2 + 2);
        left_point_ = this->height() / 2 + 2;
        if (thumb_point_ == 0) {
            thumb_point_ = left_point_;
        }

        if (selected) {
            painter.setBrush(QBrush(QColor(selected_thumb_color)));
        } else {
            painter.setBrush(QBrush(QColor(normal_thumb_color)));
        }

        painter.drawEllipse(QPoint(thumb_point_, this->height() / 2), circle_radius, circle_radius);

        if (left_point_ > 0 && right_point_ > 0 && need_repair_) {
            need_repair_ = false;
            QMetaObject::invokeMethod(this, [=, this]() {
                ExecAnimation(selected);
            });
        }
    }

    void SwitchButton::resizeEvent(QResizeEvent *event) {

    }

    void SwitchButton::enterEvent(QEnterEvent *event) {

    }

    void SwitchButton::leaveEvent(QEvent *event) {

    }

    void SwitchButton::mouseReleaseEvent(QMouseEvent *event) {
        selected = !selected;
        update();

        ExecAnimation(selected);
        if (click_cbk_) {
            click_cbk_(selected);
        }
    }

    void SwitchButton::SetStatus(bool enabled) {
        selected = enabled;
        if (thumb_point_ != 0) {
            ExecAnimation(selected);
        }
        if (left_point_ == 0 && right_point_ == 0) {
            need_repair_ = true;
        }
        qDebug() << "left : " << left_point_ << " right : " << right_point_;
    }

    void SwitchButton::ExecAnimation(bool selected) {
        if (selected) {
            // left => right
            auto anim = new QPropertyAnimation();
            anim->setStartValue(left_point_);
            anim->setEndValue(right_point_);
            connect(anim, &QPropertyAnimation::valueChanged, this, [=, this](const QVariant &value) {
                thumb_point_ = value.toInt();
                update();
            });
            connect(anim, &QPropertyAnimation::finished, this, [=]() {
                anim->deleteLater();
            });
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->setDuration(300);
            anim->start();
        } else {
            // right => left
            auto anim = new QPropertyAnimation();
            anim->setStartValue(right_point_);
            anim->setEndValue(left_point_);
            connect(anim, &QPropertyAnimation::valueChanged, this, [=, this](const QVariant &value) {
                thumb_point_ = value.toInt();
                update();
            });
            connect(anim, &QPropertyAnimation::finished, this, [=]() {
                anim->deleteLater();
            });
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->setDuration(300);
            anim->start();
        }
    }

}
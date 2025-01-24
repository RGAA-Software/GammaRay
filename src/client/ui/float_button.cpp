//
// Created by RGAA on 2023-08-22.
//

#include "float_button.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPropertyAnimation>

#include "tc_common_new/log.h"

namespace tc
{

    FloatButton::FloatButton(QWidget *parent) : QWidget(parent) {
        expand_pixmap_ = QPixmap::fromImage(QImage(":/resources/image/ic_expand.svg"));
        expand_pixmap_ = expand_pixmap_.scaled(expand_pixmap_.width() / 2, expand_pixmap_.height() / 2,
                                               Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    FloatButton::~FloatButton() {

    }

    void FloatButton::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setPen(Qt::NoPen);
        if (pressed_) {
            painter.setBrush(QBrush(QColor(0x00, 0x00, 0x00, 0xBB * transparency_)));
        } else if (enter_) {
            painter.setBrush(QBrush(QColor(0x00, 0x00, 0x00, 0x99 * transparency_)));
        } else {
            painter.setBrush(QBrush(QColor(0x00, 0x00, 0x00, 0x77 * transparency_)));
        }
        int w = QWidget::width();
        int h = QWidget::height();
        int radius = QWidget::height() / 2;
        painter.drawRoundedRect(0, 0, w, h, radius, radius);

        painter.drawPixmap((w - expand_pixmap_.width()) / 2, (h - expand_pixmap_.height()) / 2, expand_pixmap_);
    }

    void FloatButton::enterEvent(QEnterEvent *event) {
        enter_ = true;
        update();
    }

    void FloatButton::leaveEvent(QEvent *event) {
        enter_ = false;
        update();
    }

    void FloatButton::mousePressEvent(QMouseEvent *event) {
        pressed_ = true;
        update();
    }

    void FloatButton::mouseReleaseEvent(QMouseEvent *event) {
        pressed_ = false;
        update();

        if (click_cbk_) {
            click_cbk_();
        }
    }

    void FloatButton::ShowWithAnim() {
        auto animation = new QPropertyAnimation();
        animation->setDuration(350);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        connect(animation, &QPropertyAnimation::finished, this, [=]() {
            delete animation;
        });
        connect(animation, &QPropertyAnimation::valueChanged, this, [=](const QVariant &value) {
            this->transparency_ = value.toFloat();
            this->update();
        });
        animation->setEasingCurve(QEasingCurve::OutCubic);
        animation->start();
        show();
    }

    void FloatButton::HideWithAnim(std::function<void()> &&finished_task) {
        auto animation = new QPropertyAnimation(this, "windowOpacity");
        animation->setDuration(350);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);
        connect(animation, &QPropertyAnimation::finished, this, [=]() {
            if (finished_task) {
                finished_task();
            }
            this->hide();
            delete animation;
        });
        connect(animation, &QPropertyAnimation::valueChanged, this, [=](const QVariant &value) {
            this->transparency_ = value.toFloat();
            this->update();
        });
        animation->start();
    }

    void FloatButton::Show() {
        this->transparency_ = 1;
        show();
    }

    void FloatButton::Hide() {
        this->transparency_ = 0;
        hide();
    }
}
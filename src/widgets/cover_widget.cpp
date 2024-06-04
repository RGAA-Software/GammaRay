#include "cover_widget.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QRgba64>
#include <qfontmetrics.h>
#include <QPropertyAnimation>

namespace tc
{
    CoverWidget::CoverWidget(QWidget *parent, int offset) : QWidget(parent) {
        this->offset = offset;
    }

    CoverWidget::~CoverWidget() {

    }

    void CoverWidget::paintEvent(QPaintEvent *event) {
        QWidget::paintEvent(event);
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        int radius = 7;
        if (cursor_enter) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0x00, 0x00, 0x00, widget_alpha));
            painter.drawRoundedRect(offset, offset, width() - 2 * offset, height() - 2 * offset, radius, radius);
        }

        if (running_) {
            painter.setBrush(QBrush(QColor(0x00ff00)));
            painter.drawRoundedRect(5, 5, 16, 16, 8, 8);
        }
    }

    void CoverWidget::enterEvent(QEnterEvent *event) {
        cursor_enter = true;
        auto animation = new QPropertyAnimation(this, "");
        animation->setDuration(200);
        animation->setStartValue(0);
        animation->setEndValue(target_alpha);
        QObject::connect(animation, &QPropertyAnimation::valueChanged, this, [this](QVariant val) {
            widget_alpha = val.toInt();
            repaint();
        });
        QObject::connect(animation, &QPropertyAnimation::finished, this, [animation]() {
            delete animation;
        });
        animation->start();
        repaint();
    }

    void CoverWidget::leaveEvent(QEvent *event) {
        auto animation = new QPropertyAnimation(this, "");
        animation->setDuration(200);
        animation->setStartValue(target_alpha);
        animation->setEndValue(0);
        QObject::connect(animation, &QPropertyAnimation::valueChanged, this, [this](QVariant val) {
            widget_alpha = val.toInt();
            repaint();
        });
        QObject::connect(animation, &QPropertyAnimation::finished, this, [this, animation]() {
            cursor_enter = false;
            repaint();
            delete animation;
        });
        animation->start();
        repaint();
    }

    void CoverWidget::UpdateTagText(const QString &t) {
        tag_text = t;
    }

    void CoverWidget::SetTagEnableStatus(bool enable) {
        enable_tab_display = enable;
    }

    void CoverWidget::SetRunningStatus(bool running) {
        bool old_status = running_;
        this->running_ = running;
        if (old_status != this->running_) {
            repaint();
        }
    }

}
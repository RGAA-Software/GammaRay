#include "media_record_sign_lab.h"
#include <QTimer>
#include <qfont.h>
#include "tc_common_new/log.h"
#include "app_color_theme.h"

namespace tc
{
    MediaRecordSignLab::MediaRecordSignLab(QWidget* parent) : QWidget(parent) {
        setFixedSize(38, 24);
        setAttribute(Qt::WA_StyledBackground, true);
        this->setStyleSheet("background:#FFFFFFFF;");
        timer_ = new QTimer(this);
        timer_->setInterval(1000);
        brush_color_ = QColor(0xFF, 0x5E, 0x57);
        color_value_ = 0xff5357;
        connect(timer_, &QTimer::timeout, this, [=, this]() {
            update();
            ++toggle_;
        });

        timer_->start();
    }

    MediaRecordSignLab::~MediaRecordSignLab() {
        timer_->stop();
    }

    void MediaRecordSignLab::paintEvent(QPaintEvent* event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        QPen pen(0x999999);
        pen.setWidth(2);
        painter.setPen(pen);
        QPen font_pen;
        if (toggle_ % 2 == 0) {
            painter.setBrush(QBrush(0xff0000));
            font_pen.setColor(QColor(0xffffff));
        }
        else {
            painter.setBrush(QBrush(0xffffff));
            font_pen.setColor(QColor(0xff0000));
        }
        painter.drawRoundedRect(this->rect(), 2, 2);
        painter.save();
        QFont font{ "Microsoft YaHei" };
        font.setPixelSize(14);
        font.setBold(true);
        painter.setPen(font_pen);
        painter.setFont(font);
        painter.drawText(6, 17, "REC");
        painter.restore();
    }

}
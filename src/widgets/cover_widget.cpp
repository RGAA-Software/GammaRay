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
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(offset, offset, width() - 2 * offset, height() - 2 * offset, radius, radius);

//            if (wp_entity) {
//                QPen pen(QColor(0xff, 0xff, 0xff));
//                painter.setPen(pen);
//                QFontMetrics fm(title_font);
//                int width = fm.width(wp_entity->wp_name.c_str());
//                int height = fm.height();
//                painter.setFont(title_font);
//                painter.drawText((this->width() - width) / 2, this->height() - skScaleSize(70), width, height, 0,
//                                 wp_entity->wp_name.c_str());
//
//                QFontMetrics author_fm(sk::SysConfig::Instance()->sys_font_11);
//                stdstr author = "@:" + (wp_entity->wp_author_name.empty() ? "Anonymity" : wp_entity->wp_author_name);
//                painter.setFont(sk::SysConfig::Instance()->sys_font_11);
//                int author_width = author_fm.width(author.c_str());
//                int author_height = author_fm.height();
//                painter.drawText((this->width() - author_width) / 2, this->height() - skScaleSize(43), author_width,
//                                 author_height, 0, author.c_str());
//            }
        }

//        if (enable_tab_display) {
//            int tag_width = skScaleSize(80);
//            int tag_height = skScaleSize(20);
//            int x = width() - tag_width - skScaleSize(10);
//            int y = skScaleSize(10);
//            painter.setPen(Qt::NoPen);
//            painter.setBrush(QBrush(QRgba64::fromRgba(0xaa, 0xaa, 0xaa, 0xaa)));
//            painter.drawRoundedRect(x, y, tag_width, tag_height, skScaleSize(4), skScaleSize(4));
//
//            QPen pen(0xffffff);
//            pen.setWidth(skScaleSize(1));
//            painter.setPen(pen);
//            painter.setFont(sk::SysConfig::Instance()->sys_font_10);
//            QFontMetrics fm = painter.fontMetrics();
//            int width = fm.width(tag_text);
//            int height = fm.descent() + fm.ascent();
//            int font_x = x + (tag_width - width) / 2;
//            int font_y = y + (tag_height - height) / 2 + fm.ascent();
//            painter.drawText(font_x, font_y, tag_text);
//        }
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

}